/**
 * server/src/board/map/signalpath.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2023 Reinder Feenstra
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

#include "signalpath.hpp"
#include "../tile/rail/blockrailtile.hpp"
#include "../tile/rail/turnout/turnoutrailtile.hpp"
#include "../tile/rail/directioncontrolrailtile.hpp"
#include "../tile/rail/onewayrailtile.hpp"
#include "../tile/rail/linkrailtile.hpp"
#include "../map/signalpath.hpp"

std::shared_ptr<Link> otherLink(const Node& node, const Link& link)
{
  static const std::shared_ptr<Link> noLink{};
  const auto& links = node.links();
  if(links.size() == 2)
    return links[(links[0].get() == &link) ? 1 : 0];
  assert(false);
  return noLink;
}


SignalPath::SignalPath(const Node& signalNode, size_t blocksAhead, std::function<void(const std::vector<BlockState>&)> onEvaluated)
  : m_signalNode{signalNode}
  , m_onEvaluated{std::move(onEvaluated)}
{
  if(auto link = signalNode.getLink(1))
    m_root = findBlocks(signalNode, *link, blocksAhead);
  evaluate();
}

SignalPath::~SignalPath()
{
  for(auto& connection : m_connections)
    connection.disconnect();
}

void SignalPath::evaluate()
{
  std::vector<BlockState> blockStates;
  const Item* item = m_root.get();
  while(item)
  {
    if(const auto* blockItem = dynamic_cast<const BlockItem*>(item))
      blockStates.emplace_back(blockItem->blockState());
    item = item->next().get();
  }
  m_onEvaluated(blockStates);
}

std::unique_ptr<const SignalPath::Item> SignalPath::findBlocks(const Node& node, const Link& link, size_t blocksAhead)
{
  const auto& nextNode = link.getNext(node);
  auto tile = nextNode.tile().shared_ptr<Tile>();

  if(auto block = std::dynamic_pointer_cast<BlockRailTile>(tile))
  {
    m_connections.emplace_back(block->stateChanged.connect(
      [this](const BlockRailTile& /*tile*/, BlockState /*state*/)
      {
        evaluate();
      }));

    std::unique_ptr<const Item> next;
    if(blocksAhead > 1)
      if(const auto& nextLink = otherLink(nextNode, link))
        next = findBlocks(nextNode, *nextLink, blocksAhead - 1);
    return std::unique_ptr<const SignalPath::Item>{new BlockItem(block, std::move(next))};
  }
  if(auto turnout = std::dynamic_pointer_cast<TurnoutRailTile>(tile))
  {
    m_connections.emplace_back(turnout->positionChanged.connect(
      [this](const TurnoutRailTile& /*tile*/, TurnoutPosition /*position*/)
      {
        evaluate();
      }));

    std::map<TurnoutPosition, std::unique_ptr<const Item>> next;
    switch(turnout->tileId())
    {
      case TileId::RailTurnoutLeft45:
      case TileId::RailTurnoutLeft90:
      case TileId::RailTurnoutLeftCurved:
        //  1  2
        //   \ |
        //    \|
        //     0
        if(nextNode.getLink(0).get() == &link)
        {
          next.emplace(TurnoutPosition::Left, findBlocks(nextNode, nextNode.getLink(1), blocksAhead));
          next.emplace(TurnoutPosition::Straight, findBlocks(nextNode, nextNode.getLink(2), blocksAhead));
        }
        else
        {
          assert(nextNode.getLink(1).get() == &link || nextNode.getLink(2).get() == &link);
          next.emplace(nextNode.getLink(1).get() == &link ? TurnoutPosition::Left : TurnoutPosition::Straight, findBlocks(nextNode, nextNode.getLink(0), blocksAhead));
        }
        break;

      case TileId::RailTurnoutRight45:
      case TileId::RailTurnoutRight90:
      case TileId::RailTurnoutRightCurved:
        //  1  2
        //  | /
        //  |/
        //  0
        if(nextNode.getLink(0).get() == &link)
        {
          next.emplace(TurnoutPosition::Straight, findBlocks(nextNode, nextNode.getLink(1), blocksAhead));
          next.emplace(TurnoutPosition::Right, findBlocks(nextNode, nextNode.getLink(2), blocksAhead));
        }
        else
        {
          assert(nextNode.getLink(1).get() == &link || nextNode.getLink(2).get() == &link);
          next.emplace(nextNode.getLink(1).get() == &link ? TurnoutPosition::Straight : TurnoutPosition::Right, findBlocks(nextNode, nextNode.getLink(0), blocksAhead));
        }
        break;

      case TileId::RailTurnoutWye:
        //  1     2
        //   \   /
        //    \ /
        //     0
        if(nextNode.getLink(0).get() == &link)
        {
          next.emplace(TurnoutPosition::Left, findBlocks(nextNode, nextNode.getLink(1), blocksAhead));
          next.emplace(TurnoutPosition::Right, findBlocks(nextNode, nextNode.getLink(2), blocksAhead));
        }
        else
        {
          assert(nextNode.getLink(1).get() == &link || nextNode.getLink(2).get() == &link);
          next.emplace(nextNode.getLink(1).get() == &link ? TurnoutPosition::Left : TurnoutPosition::Right, findBlocks(nextNode, nextNode.getLink(0), blocksAhead));
        }
        break;

      case TileId::RailTurnout3Way:
        //  1  2  3
        //   \ | /
        //    \|/
        //     0
        if(nextNode.getLink(0).get() == &link)
        {
          next.emplace(TurnoutPosition::Left, findBlocks(nextNode, nextNode.getLink(1), blocksAhead));
          next.emplace(TurnoutPosition::Straight, findBlocks(nextNode, nextNode.getLink(2), blocksAhead));
          next.emplace(TurnoutPosition::Right, findBlocks(nextNode, nextNode.getLink(3), blocksAhead));
        }
        else
        {
          TurnoutPosition position = TurnoutPosition::Unknown;
          if(nextNode.getLink(1).get() == &link)
            position = TurnoutPosition::Left;
          else if(nextNode.getLink(2).get() == &link)
            position = TurnoutPosition::Straight;
          else if(nextNode.getLink(3).get() == &link)
            position = TurnoutPosition::Right;

          assert(position != TurnoutPosition::Unknown);
          next.emplace(position, findBlocks(nextNode, nextNode.getLink(0), blocksAhead));
        }
        break;

      case TileId::RailTurnoutDoubleSlip:
      case TileId::RailTurnoutSingleSlip:
        //  Double:      Single:
        //      2            2
        //      |\           |
        //  1 --+-- 3    1 --+-- 3
        //     \|           \|
        //      0            0
        if(nextNode.getLink(0).get() == &link)
        {
          next.emplace(TurnoutPosition::Crossed, findBlocks(nextNode, nextNode.getLink(2), blocksAhead));
          next.emplace(TurnoutPosition::Diverged, findBlocks(nextNode, nextNode.getLink(1), blocksAhead));
        }
        else if(nextNode.getLink(1).get() == &link)
        {
          next.emplace(TurnoutPosition::Crossed, findBlocks(nextNode, nextNode.getLink(3), blocksAhead));
          next.emplace(TurnoutPosition::Diverged, findBlocks(nextNode, nextNode.getLink(0), blocksAhead));
        }
        else if(nextNode.getLink(2).get() == &link)
        {
          next.emplace(TurnoutPosition::Crossed, findBlocks(nextNode, nextNode.getLink(0), blocksAhead));
          if(turnout->tileId() == TileId::RailTurnoutDoubleSlip)
            next.emplace(TurnoutPosition::Diverged, findBlocks(nextNode, nextNode.getLink(3), blocksAhead));
        }
        else if(nextNode.getLink(3).get() == &link)
        {
          next.emplace(TurnoutPosition::Crossed, findBlocks(nextNode, nextNode.getLink(1), blocksAhead));
          if(turnout->tileId() == TileId::RailTurnoutDoubleSlip)
            next.emplace(TurnoutPosition::Diverged, findBlocks(nextNode, nextNode.getLink(2), blocksAhead));
        }
        else
        {
          assert(false);
          return {};
        }
        break;

      default:
        assert(false);
        return {};
    }

    if(!next.empty())
      return std::unique_ptr<const SignalPath::Item>{new TurnoutItem(turnout, std::move(next))};
  }
  else if(auto direction = std::dynamic_pointer_cast<DirectionControlRailTile>(tile))
  {
    if(const auto& nextLink = otherLink(nextNode, link))
    {
      //  1 B
      //   |
      //  ( )
      //   |
      //  0 A
      m_connections.emplace_back(direction->stateChanged.connect(
        [this](const DirectionControlRailTile& /*tile*/, DirectionControlState /*state*/)
        {
          evaluate();
        }));

      return std::unique_ptr<const SignalPath::Item>{
        new DirectionControlItem(
          direction,
          nextNode.getLink(0).get() == &link ? DirectionControlState::AtoB : DirectionControlState::BtoA,
          findBlocks(nextNode, *nextLink, blocksAhead))};
    }
  }
  else if(std::dynamic_pointer_cast<OneWayRailTile>(tile))
  {
    //  1
    //  |
    //  ^
    //  |
    //  0
    if(const auto& nextLink = otherLink(nextNode, link))
      if(nextNode.getLink(0).get() == &link)
        return findBlocks(nextNode, *nextLink, blocksAhead);
  }
  else if(isRailBridge(tile->tileId()) || isRailCross(tile->tileId()))
  {
    //     2      1 2      2 3
    //     |       \|      |/
    // 1 --+-- 3    |      |
    //     |        |\    /|
    //     0        0 3  1 0
    static const std::array<std::pair<size_t, size_t>, 4> opposite{{{0, 2}, {1, 3}, {2, 0}, {3, 1}}};
    for(auto p : opposite)
      if(nextNode.getLink(p.first).get() == &link)
        return findBlocks(nextNode, nextNode.getLink(p.second), blocksAhead);
    assert(false);
  }
  else if(auto linkTile = std::dynamic_pointer_cast<LinkRailTile>(tile))
  {
    if(linkTile->link)
      if(auto linkNode = linkTile->link->node())
        return findBlocks(linkNode->get(), linkNode->get().getLink(0), blocksAhead);
  }
  else if(tile->tileId() != TileId::RailBufferStop)
  {
    if(const auto& nextLink = otherLink(nextNode, link))
    {
      if(&nextNode == &m_signalNode)
        return {}; // we're reached oursels

      return findBlocks(nextNode, *nextLink, blocksAhead);
    }
    assert(false); // unhandled rail tile
  }

  return {};
}


BlockState SignalPath::BlockItem::blockState() const
{
  if(auto block = m_block.lock())
    return block->state;
  return BlockState::Unknown;
}

const std::unique_ptr<const SignalPath::Item>& SignalPath::DirectionControlItem::next() const
{
  if(auto directionControl = m_directionControl.lock())
  {
    const auto state = directionControl->state.value();
    if(state == DirectionControlState::Both || state == m_oneWayState)
      return m_next;
  }
  return noItem;
}

const std::unique_ptr<const SignalPath::Item>& SignalPath::TurnoutItem::next() const
{
  if(auto turnout = m_turnout.lock())
    if(auto it = m_next.find(turnout->position.value()); it != m_next.end())
      return it->second;
  return noItem;
}
