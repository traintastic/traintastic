/**
 * server/src/board/map/abstractsignalpath.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2024 Reinder Feenstra
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

#include "abstractsignalpath.hpp"
#include "../../core/objectproperty.tpp"
#include "../../core/method.tpp"
#include "../tile/rail/blockrailtile.hpp"
#include "../tile/rail/turnout/turnoutrailtile.hpp"
#include "../tile/rail/directioncontrolrailtile.hpp"
#include "../tile/rail/onewayrailtile.hpp"
#include "../tile/rail/linkrailtile.hpp"
#include "../tile/rail/signal/signalrailtile.hpp"
#include "../map/abstractsignalpath.hpp"
#include "../../train/train.hpp" // FIXME: required due to forward declaration

AbstractSignalPath::AbstractSignalPath(SignalRailTile& signal)
  : m_signal{signal}
{
}

AbstractSignalPath::AbstractSignalPath(SignalRailTile& signal, size_t blocksAhead)//, std::function<void(const std::vector<BlockState>&)> onEvaluated)
  : AbstractSignalPath(signal)
{
  const auto& signalNode = signal.node()->get();
  if(auto link = signalNode.getLink(1); link && blocksAhead != 0)
    m_root = findBlocks(signalNode, *link, blocksAhead);

  {
    // Require a reserved path if there is at least one turnout in the path to the next block.
    // NOTE: this can be overriden using the signal's requireReservation property.
    const AbstractSignalPath::Item* item = m_root.get();
    while(item)
    {
      if(dynamic_cast<const BlockItem*>(item))
      {
        break;
      }
      if(dynamic_cast<const TurnoutItem*>(item))
      {
        m_requireReservation = true;
        break;
      }
      item = item->next().get();
    }
  }
}

AbstractSignalPath::~AbstractSignalPath()
{
  for(auto& connection : m_connections)
  {
    connection.disconnect();
  }
}

void AbstractSignalPath::evaluate()
{
  const bool stop = !signal().hasReservedPath() && requireReservation();

  setAspect(stop ? SignalAspect::Stop : determineAspect());
}

bool AbstractSignalPath::requireReservation() const
{
  return (m_signal.requireReservation == AutoYesNo::Yes || (m_signal.requireReservation == AutoYesNo::Auto && m_requireReservation));
}

const AbstractSignalPath::BlockItem* AbstractSignalPath::nextBlock(const Item* item) const
{
  while(item)
  {
    if(const auto* blockItem = dynamic_cast<const BlockItem*>(item))
    {
      return blockItem;
    }
    item = item->next().get();
  }
  return nullptr;
}

std::tuple<const AbstractSignalPath::BlockItem*, const AbstractSignalPath::SignalItem*> AbstractSignalPath::nextBlockOrSignal(const Item* item) const
{
  while(item)
  {
    if(const auto* blockItem = dynamic_cast<const BlockItem*>(item))
    {
      return {blockItem, nullptr};
    }
    if(const auto* signalItem = dynamic_cast<const SignalItem*>(item))
    {
      return {nullptr, signalItem};
    }
    item = item->next().get();
  }
  return {nullptr, nullptr};
}

void AbstractSignalPath::getBlockStates(tcb::span<BlockState> blockStates) const
{
  size_t i = 0;
  const Item* item = m_root.get();
  while(item && i < blockStates.size())
  {
    if(const auto* blockItem = dynamic_cast<const BlockItem*>(item))
    {
      blockStates[i] = blockItem->blockState();
      i++;
    }
    item = item->next().get();
  }

  if(i < blockStates.size())
  {
    std::fill(blockStates.data() + i, blockStates.data() + blockStates.size(), BlockState::Unknown);
  }
}

std::shared_ptr<BlockRailTile> AbstractSignalPath::getBlock(size_t index) const
{
  const Item* item = m_root.get();
  while(item)
  {
    if(const auto* blockItem = dynamic_cast<const BlockItem*>(item))
    {
      if(index == 0)
      {
        return blockItem->block();
      }
      index--;
    }
    item = item->next().get();
  }
  return {};
}

std::unique_ptr<const AbstractSignalPath::Item> AbstractSignalPath::findBlocks(const Node& node, const Link& link, size_t blocksAhead)
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

    const auto enterSide = (nextNode.getLink(0).get() == &link) ? BlockSide::A : BlockSide::B;
    std::unique_ptr<const Item> next;
    if(blocksAhead > 1)
      if(const auto& nextLink = otherLink(nextNode, link))
        next = findBlocks(nextNode, *nextLink, blocksAhead - 1);
    return std::unique_ptr<const AbstractSignalPath::Item>{new BlockItem(block, enterSide, std::move(next))};
  }
  if(auto signal = std::dynamic_pointer_cast<SignalRailTile>(tile))
  {
    if(const auto& nextLink = otherLink(nextNode, link))
    {
      m_connections.emplace_back(signal->aspectChanged.connect(
        [this](const SignalRailTile& /*tile*/, SignalAspect /*aspect*/)
        {
          evaluate();
        }));

      return std::unique_ptr<const AbstractSignalPath::Item>{
        new SignalItem(
          signal,
          findBlocks(nextNode, *nextLink, blocksAhead))};
    }
  }
  else if(auto turnout = std::dynamic_pointer_cast<TurnoutRailTile>(tile))
  {
    m_connections.emplace_back(turnout->positionChanged.connect(
      [this](const TurnoutRailTile& /*tile*/, TurnoutPosition /*position*/)
      {
        evaluate();
      }));

    std::map<TurnoutPosition, std::unique_ptr<const Item>> next;
    for (const auto& tpl : getTurnoutLinks(*turnout, link))
    {
      next.emplace(tpl.turnoutPosition, findBlocks(nextNode, nextNode.getLink(tpl.linkIndex), blocksAhead));
    }

    if(!next.empty())
      return std::unique_ptr<const AbstractSignalPath::Item>{new TurnoutItem(turnout, std::move(next))};
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

      return std::unique_ptr<const AbstractSignalPath::Item>{
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
  else if(isRailBridge(tile->tileId) || isRailCross(tile->tileId))
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
  else if(tile->tileId != TileId::RailBufferStop)
  {
    if(const auto& nextLink = otherLink(nextNode, link))
    {
      if(&nextNode == &m_signal.node()->get())
        return {}; // we're reached oursels

      return findBlocks(nextNode, *nextLink, blocksAhead);
    }
    assert(false); // unhandled rail tile
  }

  return {};
}

void AbstractSignalPath::setAspect(SignalAspect value) const
{
  m_signal.setAspect(value);
}


std::shared_ptr<BlockRailTile> AbstractSignalPath::BlockItem::block() const noexcept
{
  return m_block.lock();
}

BlockState AbstractSignalPath::BlockItem::blockState() const
{
  if(auto blk = block())
  {
    return blk->state;
  }
  return BlockState::Unknown;
}

const std::unique_ptr<const AbstractSignalPath::Item>& AbstractSignalPath::DirectionControlItem::next() const
{
  if(auto directionControl = m_directionControl.lock())
  {
    const auto state = directionControl->state.value();
    if(state == DirectionControlState::Both || state == m_oneWayState)
      return m_next;
  }
  return noItem;
}

const std::unique_ptr<const AbstractSignalPath::Item>& AbstractSignalPath::TurnoutItem::next() const
{
  if(auto turnout = m_turnout.lock())
    if(auto it = m_next.find(turnout->position.value()); it != m_next.end())
      return it->second;
  return noItem;
}
