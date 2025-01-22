/**
 * server/src/board/tile/rail/nxbuttonrailtile.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023-2024 Reinder Feenstra
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

#include "nxbuttonrailtile.hpp"
#include "blockrailtile.hpp"
#include "../../map/link.hpp"
#include "../../nx/nxmanager.hpp"
#include "../../../core/attributes.hpp"
#include "../../../core/objectproperty.tpp"
#include "../../../log/log.hpp"
#include "../../../utils/displayname.hpp"
#include "../../../world/world.hpp"

static std::shared_ptr<BlockRailTile> findBlock(Node& node, uint8_t linkIndex)
{
  auto link = node.getLink(linkIndex);
  if(!link)
  {
    return {};
  }
  auto* tile = &link->getNext(node).tile();
  while(tile->tileId != TileId::RailBlock)
  {
    if(isRailBridge(tile->tileId))
    {
      auto& bridgeNode = (*tile->node()).get();
      size_t index = bridgeNode.getIndex(*link);
      if(index >= bridgeNode.links().size()) /*[[unlikely]]*/
      {
        assert(false);
        return {};
      }
      link = bridgeNode.getLink((index + 2) % 4);
      tile = &link->getNext(bridgeNode).tile();
    }
    else
    {
      return {};
    }
  }
  return tile->shared_ptr<BlockRailTile>();
}

NXButtonRailTile::NXButtonRailTile(World& world, std::string_view id_)
  : StraightRailTile(world, id_, TileId::RailNXButton)
  , m_node{*this, 2}
  , name{this, "name", std::string{id_}, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , enabled{this, "enabled", false, PropertyFlags::ReadOnly | PropertyFlags::NoStore}
  , block{this, "block", nullptr, PropertyFlags::ReadOnly | PropertyFlags::NoStore}
  , input{this, "input", nullptr, PropertyFlags::ReadWrite | PropertyFlags::Store, nullptr,
      [this](const std::shared_ptr<Input>& value)
      {
        if(input)
        {
          disconnectInput(*input);
        }
        if(value)
        {
          connectInput(*value);
        }
        return true;
      }}
{
  const bool editable = contains(m_world.state, WorldState::Edit);

  Attributes::addEnabled(name, editable);
  Attributes::addDisplayName(name, DisplayName::Object::name);
  m_interfaceItems.add(name);

  Attributes::addObjectEditor(enabled, false);
  m_interfaceItems.add(enabled);

  m_interfaceItems.add(block);

  Attributes::addEnabled(input, editable);
  //Attributes::addDisplayName(input, DisplayName::Hardware::input);
  Attributes::addObjectList(input, world.inputs);
  m_interfaceItems.add(input);
}

NXButtonRailTile::~NXButtonRailTile()
{
  assert(!input);
  assert(!m_inputValueChanged.connected());
  assert(!m_inputDestroying.connected());
}

void NXButtonRailTile::loaded()
{
  StraightRailTile::loaded();

  if(input)
    connectInput(*input);

  updateEnabled();
}

void NXButtonRailTile::destroying()
{
  input = nullptr;
  StraightRailTile::destroying();
}

void NXButtonRailTile::worldEvent(WorldState worldState, WorldEvent worldEvent)
{
  StraightRailTile::worldEvent(worldState, worldEvent);

  const bool editable = contains(worldState, WorldState::Edit);

  Attributes::setEnabled(name, editable);
  Attributes::setEnabled(input, editable);

  updateEnabled();
}

void NXButtonRailTile::boardModified()
{
  const auto blockA = findBlock(m_node, 0);
  const auto blockB = findBlock(m_node, 1);

  if(blockA && !blockB) // conencted to block A
  {
    block.setValueInternal(blockA);
  }
  else if(!blockA && blockB) // conencted to block B
  {
    block.setValueInternal(blockB);
  }
  else if(blockA && blockB) // connected to two blocks
  {
    block.setValueInternal(nullptr);
    Log::log(*this, LogMessage::W3001_NX_BUTTON_CONNECTED_TO_TWO_BLOCKS);
  }
  else // not connected to any block
  {
    block.setValueInternal(nullptr);
    Log::log(*this, LogMessage::W3002_NX_BUTTON_NOT_CONNECTED_TO_ANY_BLOCK);
  }

  updateEnabled();
}

void NXButtonRailTile::connectInput(Input& object)
{
  object.consumers.appendInternal(shared_from_this());
  m_inputDestroying = object.onDestroying.connect(
    [this]([[maybe_unused]] Object& obj)
    {
      assert(input.value().get() == &obj);
      input = nullptr;
    });
  m_inputValueChanged = object.onValueChanged.connect(
    [this](bool value, const std::shared_ptr<Input>& /*input*/)
    {
      if(!enabled)
      {
        return; // not enabled, no action
      }

      if(value)
      {
        m_world.nxManager->pressed(*this);
      }
      else
      {
        m_world.nxManager->released(*this);
      }
    });
}

void NXButtonRailTile::disconnectInput(Input& object)
{
  m_inputValueChanged.disconnect();
  m_inputDestroying.disconnect();
  object.consumers.removeInternal(shared_from_this());
}

void NXButtonRailTile::updateEnabled()
{
  enabled.setValueInternal(block && contains(m_world.state, WorldState::Run));
}
