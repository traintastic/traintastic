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
#include "../tile/rail/bridgerailtile.hpp"
#include "../tile/rail/crossrailtile.hpp"
#include "../tile/rail/directioncontrolrailtile.hpp"
#include "../tile/rail/signal/signalrailtile.hpp"
#include "../tile/rail/turnout/turnoutrailtile.hpp"
#include "../tile/rail/linkrailtile.hpp"
#include "../tile/rail/nxbuttonrailtile.hpp"
#include "../../core/objectproperty.tpp"
#include "../../enum/bridgepath.hpp"

template<class T1, typename T2>
static bool contains(const std::vector<std::pair<std::weak_ptr<T1>, T2>>& values, const std::shared_ptr<T1>& value)
{
  const auto it = std::find_if(values.begin(), values.end(),
    [&value](const auto& item)
    {
      return item.first.lock() == value;
    });
  return it != values.end();
}

std::vector<std::shared_ptr<BlockPath>> BlockPath::find(BlockRailTile& startBlock)
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
    std::shared_ptr<BlockPath> path;
    const Node* node;
    const Link* link;
  };

  std::vector<std::shared_ptr<BlockPath>> paths;

  std::queue<Position> todo;
  if(linkA)
  {
    todo.emplace(Position{std::make_shared<BlockPath>(startBlock, BlockSide::A), &node, linkA.get()});
  }
  if(linkB)
  {
    todo.emplace(Position{std::make_shared<BlockPath>(startBlock, BlockSide::B), &node, linkB.get()});
  }

  while(!todo.empty())
  {
    auto& current = todo.front();
    if(!current.link)
    {
      todo.pop(); // drop it, dead end
      continue;
    }

    for(const auto& tile : current.link->tiles()) // add passive tiles to reserve
    {
      current.path->m_tiles.emplace_back(std::static_pointer_cast<RailTile>(tile));
    }

    assert(current.node);
    const auto& nextNode = current.link->getNext(*current.node);
    auto& tile = nextNode.tile();

    switch(tile.tileId())
    {
      case TileId::RailBlock:
      {
        if(current.node->tile().tileId() == TileId::RailNXButton)
        {
          current.path->m_nxButtonTo = current.node->tile().shared_ptr<NXButtonRailTile>();
        }

        auto& block = static_cast<BlockRailTile&>(tile);
        current.path->m_toBlock = block.shared_ptr<BlockRailTile>();
        current.path->m_toSide = nextNode.getLink(0).get() == current.link ? BlockSide::A : BlockSide::B;
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
        auto turnout = tile.shared_ptr<TurnoutRailTile>();
        if(contains(current.path->m_turnouts, turnout))
        {
          todo.pop(); // drop it, can't pass turnout twice
          break;
        }
        auto links = getTurnoutLinks(*turnout, *current.link);
        assert(!links.empty());

        if(links.size() > 1)
        {
          for(size_t i = 1; i < links.size(); ++i)
          {
            auto path = std::make_shared<BlockPath>(*current.path); // "fork" path
            path->m_turnouts.emplace_back(turnout, links[i].turnoutPosition);
            todo.emplace(Position{std::move(path), &nextNode, nextNode.getLink(links[i].linkIndex).get()});
          }
        }

        current.node = &nextNode;
        current.link = nextNode.getLink(links[0].linkIndex).get();
        current.path->m_turnouts.emplace_back(turnout, links[0].turnoutPosition);
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
          current.path->m_tiles.emplace_back(tile.shared_ptr<RailTile>());
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
            current.path->m_bridges.emplace_back(tile.shared_ptr<BridgeRailTile>(), i % 2 == 0 ? BridgePath::AC : BridgePath::BD);
            break;
          }
        }
        break;

      case TileId::RailCross45:
      case TileId::RailCross90:
      {
        //     2        2 3
        //     |        |/
        // 1 --+-- 3    |
        //     |       /|
        //     0      1 0
        auto cross = tile.shared_ptr<CrossRailTile>();
        if(contains(current.path->m_crossings, cross))
        {
          todo.pop(); // drop it, can't pass crossing twice
          break;
        }

        for(size_t i = 0; i < 4; i++)
        {
          if(nextNode.getLink(i).get() == current.link)
          {
            current.node = &nextNode;
            current.link = nextNode.getLink((i + 2) % 4).get(); // opposite
            current.path->m_crossings.emplace_back(cross, i % 2 == 0 ? CrossState::AC : CrossState::BD);
            break;
          }
        }
        break;
      }
      case TileId::RailLink:
      {
        auto& linkTile = static_cast<LinkRailTile&>(tile);
        if(linkTile.link) // is connected to another link
        {
          current.path->m_tiles.emplace_back(linkTile.shared_ptr<RailTile>());
          current.path->m_tiles.emplace_back(linkTile.link->shared_ptr<RailTile>());
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
          current.path->m_tiles.emplace_back(tile.shared_ptr<RailTile>());
        }
        break;

      case TileId::RailDecoupler:
        current.path->m_tiles.emplace_back(tile.shared_ptr<RailTile>());
        current.node = &nextNode;
        current.link = otherLink(nextNode, *current.link).get();
        break;

      case TileId::RailNXButton:
        if(&current.node->tile() == &startBlock)
        {
          current.path->m_nxButtonFrom = tile.shared_ptr<NXButtonRailTile>();
        }
        else
        {
          current.path->m_tiles.emplace_back(tile.shared_ptr<RailTile>());
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


BlockPath::BlockPath(BlockRailTile& block, BlockSide side)
  : m_fromBlock{block}
  , m_fromSide{side}
  , m_toSide{static_cast<BlockSide>(-1)}
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

bool BlockPath::reserve(const std::shared_ptr<Train>& train, bool dryRun)
{
  if(!dryRun && !reserve(train, true)) // dry run first, to make sure it will succeed (else we need rollback support)
  {
    return false;
  }

  if(!m_fromBlock.reserve(train, m_fromSide, dryRun))
  {
    assert(dryRun);
    return false;
  }

  if(auto toBlock = m_toBlock.lock()) /*[[likely]]*/
  {
    if(!toBlock->reserve(train, m_toSide, dryRun))
    {
      assert(dryRun);
      return false;
    }
  }
  else
  {
    return false;
  }

  for(const auto& [turnoutWeak, position] : m_turnouts)
  {
    if(auto turnout = turnoutWeak.lock())
    {
      if(!turnout->reserve(position, dryRun))
      {
        assert(dryRun);
        return false;
      }
    }
    else /*[[unlikely]]*/
    {
      assert(dryRun);
      return false;
    }
  }

  for(const auto& [directionControlWeak, state] : m_directionControls)
  {
    if(auto directionControl = directionControlWeak.lock())
    {
      if(!directionControl->reserve(state, dryRun))
      {
        assert(dryRun);
        return false;
      }
    }
    else /*[[unlikely]]*/
    {
      assert(dryRun);
      return false;
    }
  }

  for(const auto& [crossWeak, state] : m_crossings)
  {
    if(auto cross = crossWeak.lock())
    {
      if(!cross->reserve(state, dryRun))
      {
        assert(dryRun);
        return false;
      }
    }
    else /*[[unlikely]]*/
    {
      assert(dryRun);
      return false;
    }
  }

  for(const auto& [bridgeWeak, path] : m_bridges)
  {
    if(auto bridge = bridgeWeak.lock())
    {
      if(!bridge->reserve(path, dryRun))
      {
        assert(dryRun);
        return false;
      }
    }
    else /*[[unlikely]]*/
    {
      assert(dryRun);
      return false;
    }
  }

  for(const auto& signalWeak : m_signals)
  {
    if(auto signal = signalWeak.lock())
    {
      if(!signal->reserve(shared_from_this(), dryRun))
      {
        assert(dryRun);
        return false;
      }
    }
    else /*[[unlikely]]*/
    {
      assert(dryRun);
      return false;
    }
  }

  if(!dryRun)
  {
    for(const auto& tileWeak : m_tiles)
    {
      if(auto tile = tileWeak.lock()) /*[[likely]]*/
      {
        static_cast<RailTile&>(*tile).reserve();
      }
    }

    if(auto nxButton = m_nxButtonFrom.lock())
    {
      nxButton->reserve();
    }

    if(auto nxButton = m_nxButtonTo.lock())
    {
      nxButton->reserve();
    }
  }

  return true;
}
