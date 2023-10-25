/**
 * server/src/board/map/blockpath.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "blockpath.hpp"
#include <queue>
#include <traintastic/enum/crossstate.hpp>
#include "node.hpp"
#include "link.hpp"
#include "../tile/rail/blockrailtile.hpp"
#include "../tile/rail/crossrailtile.hpp"
#include "../tile/rail/directioncontrolrailtile.hpp"
#include "../tile/rail/signal/signalrailtile.hpp"
#include "../tile/rail/turnout/turnoutrailtile.hpp"
#include "../tile/rail/linkrailtile.hpp"
#include "../tile/rail/nxbuttonrailtile.hpp"
#include "../../core/objectproperty.tpp"

std::vector<std::unique_ptr<BlockPath>> BlockPath::find(BlockRailTile& startBlock)
{
  const auto& node = startBlock.node()->get();
  const auto& linkA = node.getLink(0);
  const auto& linkB = node.getLink(1);

  if(!linkA && !linkB)
  {
    return {};
  }

  struct Position
  {
    std::unique_ptr<BlockPath> path;
    const Node* node;
    const Link* link;
  };

  std::vector<std::unique_ptr<BlockPath>> paths;

  std::queue<Position> todo;
  if(linkA)
  {
    todo.emplace(Position{std::make_unique<BlockPath>(startBlock, Side::A), &node, linkA.get()});
  }
  if(linkB)
  {
    todo.emplace(Position{std::make_unique<BlockPath>(startBlock, Side::B), &node, linkB.get()});
  }

  while(!todo.empty())
  {
    auto& current = todo.front();
    if(!current.link)
    {
      todo.pop(); // drop it, dead end
      continue;
    }
    assert(current.node);
    const auto& nextNode = current.link->getNext(*current.node);
    auto& tile = nextNode.tile();

    switch(tile.tileId())
    {
      case TileId::RailBlock:
      {
        // temp dummy use to fix warnings:
        (void)current.path->m_fromBlock;
        (void)current.path->m_fromSide;

        if(current.node->tile().tileId() == TileId::RailNXButton)
        {
          current.path->m_nxButtonTo = current.node->tile().shared_ptr<NXButtonRailTile>();
        }

        auto& block = static_cast<BlockRailTile&>(tile);
        current.path->m_toBlock = block.shared_ptr<BlockRailTile>();
        current.path->m_toSide = nextNode.getLink(0).get() == current.link ? Side::A : Side::B;
        paths.emplace_back(std::move(current.path));
        todo.pop(); // complete
        break;
      }
      case TileId::RailTurnoutLeft45:
      case TileId::RailTurnoutLeft90:
      case TileId::RailTurnoutLeftCurved:
      case TileId::RailTurnoutRight45:
      case TileId::RailTurnoutRight90:
      case TileId::RailTurnoutRightCurved:
      case TileId::RailTurnoutWye:
      case TileId::RailTurnout3Way:
      case TileId::RailTurnoutSingleSlip:
      case TileId::RailTurnoutDoubleSlip:
      {
        auto* turnout = static_cast<TurnoutRailTile*>(&tile);
        auto links = getTurnoutLinks(*turnout, *current.link);
        assert(!links.empty());

        if(links.size() > 1)
        {
          for(size_t i = 1; i < links.size(); ++i)
          {
            auto path = std::make_unique<BlockPath>(*current.path); // "fork" path
            path->m_turnouts.emplace_back(turnout->shared_ptr<TurnoutRailTile>(), links[i].turnoutPosition);
            todo.emplace(Position{std::move(path), &nextNode, nextNode.getLink(links[i].linkIndex).get()});
          }
        }

        current.node = &nextNode;
        current.link = nextNode.getLink(links[0].linkIndex).get();
        current.path->m_turnouts.emplace_back(turnout->shared_ptr<TurnoutRailTile>(), links[0].turnoutPosition);
        break;
      }
      case TileId::RailOneWay:
        //  1
        //  |
        //  ^
        //  |
        //  0
        if(nextNode.getLink(0).get() == current.link) // 0 -> 1 = allowed
        {
          current.node = &nextNode;
          current.link = nextNode.getLink(1).get();
        }
        else // 1 -> 0 = blocked
        {
          todo.pop(); // drop it, one way only
        }
        break;

      case TileId::RailDirectionControl:
      {
        //  1 B
        //   |
        //  ( )
        //   |
        //  0 A
        const bool sideA = nextNode.getLink(0).get() == current.link;
        current.node = &nextNode;
        current.link = nextNode.getLink(sideA ? 1 : 0).get();
        current.path->m_directionControls.emplace_back(tile.shared_ptr<DirectionControlRailTile>(), sideA ? DirectionControlState::AtoB : DirectionControlState::BtoA);
        break;
      }
      case TileId::RailBridge45Left:
      case TileId::RailBridge45Right:
      case TileId::RailBridge90:
        //     2      1 2      2 3
        //     |       \|      |/
        // 1 --+-- 3    |      |
        //     |        |\    /|
        //     0        0 3  1 0
        for(size_t i = 0; i < 4; i++)
        {
          if(nextNode.getLink(i).get() == current.link)
          {
            current.node = &nextNode;
            current.link = nextNode.getLink((i + 2) % 4).get(); // opposite
            break;
          }
        }
        break;

      case TileId::RailCross45:
      case TileId::RailCross90:
        //     2        2 3
        //     |        |/
        // 1 --+-- 3    |
        //     |       /|
        //     0      1 0
        for(size_t i = 0; i < 4; i++)
        {
          if(nextNode.getLink(i).get() == current.link)
          {
            current.node = &nextNode;
            current.link = nextNode.getLink((i + 2) % 4).get(); // opposite
            current.path->m_crossings.emplace_back(tile.shared_ptr<CrossRailTile>(), i % 2 == 0 ? CrossState::AC : CrossState::BD);
            break;
          }
        }
        break;

      case TileId::RailLink:
      {
        auto& linkTile = static_cast<LinkRailTile&>(tile);
        if(linkTile.link) // is connected to another link
        {
          assert(linkTile.link->node());
          auto& linkNode = linkTile.link->node()->get();
          current.node = &linkNode;
          current.link = linkNode.getLink(0).get();
        }
        else
        {
          todo.pop(); // drop it, no connection
        }
        break;
      }
      case TileId::RailSignal2Aspect:
      case TileId::RailSignal3Aspect:
        current.node = &nextNode;
        if(nextNode.getLink(0).get() == current.link) // 0 -> 1 = frontside of signal
        {
          current.link = nextNode.getLink(1).get();
          current.path->m_signals.emplace_back(tile.shared_ptr<SignalRailTile>());
        }
        else // 1 -> 0 = backside of signal, just pass
        {
          current.link = nextNode.getLink(0).get();
        }
        break;

      case TileId::RailDecoupler:
        current.node = &nextNode;
        current.link = otherLink(nextNode, *current.link).get();
        break;

      case TileId::RailNXButton:
        if(&current.node->tile() == &startBlock)
        {
          current.path->m_nxButtonFrom = nextNode.tile().shared_ptr<NXButtonRailTile>();
        }
        current.node = &nextNode;
        current.link = otherLink(nextNode, *current.link).get();
        break;

      default: // passive or non rail tiles
        assert(false); // this should never happen
        todo.pop(); // drop it in case it does, however that is a bug!
        break;
    }
  }
  return paths;
}


BlockPath::BlockPath(BlockRailTile& block, Side side)
  : m_fromBlock{block}
  , m_fromSide{side}
  , m_toSide{static_cast<Side>(-1)}
{
}

std::shared_ptr<NXButtonRailTile> BlockPath::nxButtonFrom() const
{
  return m_nxButtonFrom.lock();
}

std::shared_ptr<NXButtonRailTile> BlockPath::nxButtonTo() const
{
  return m_nxButtonTo.lock();
}
