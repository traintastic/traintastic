/**
 * server/src/hardware/decoder/list/decoderlist.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2023 Reinder Feenstra
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

#include "decoderlist.hpp"
#include "decoderlisttablemodel.hpp"
#include "../../../world/getworld.hpp"
#include "../../../core/attributes.hpp"
#include "../../../utils/displayname.hpp"
#include "../../../utils/ifndefndebug.hpp"

DecoderList::DecoderList(Object& _parent, std::string_view parentPropertyName, DecoderListColumn _columns) :
  ObjectList<Decoder>(_parent, parentPropertyName),
  columns{_columns},
  add{*this, "add",
    [this]()
    {
      auto& world = getWorld(parent());
      auto decoder = Decoder::create(world, world.getUniqueId("decoder"));
      if(const auto controller = std::dynamic_pointer_cast<DecoderController>(parent().shared_from_this()))
      {
        // todo: select free address?
        decoder->interface = controller;
      }
      return decoder;
    }}
  , remove{*this, "remove", std::bind(&DecoderList::removeMethodHandler, this, std::placeholders::_1)}
{
  const bool editable = contains(getWorld(parent()).state.value(), WorldState::Edit);

  Attributes::addDisplayName(add, DisplayName::List::add);
  Attributes::addEnabled(add, editable);
  m_interfaceItems.add(add);

  Attributes::addDisplayName(remove, DisplayName::List::remove);
  Attributes::addEnabled(remove, editable);
  m_interfaceItems.add(remove);
}

TableModelPtr DecoderList::getModel()
{
  return std::make_shared<DecoderListTableModel>(*this);
}

std::shared_ptr<Decoder> DecoderList::getDecoder(uint16_t address) const
{
  auto it = std::find_if(begin(), end(),
    [address](const auto& decoder)
    {
      return decoder->address.value() == address;
    });
  if(it != end())
    return *it;
  return {};
}

std::shared_ptr<Decoder> DecoderList::getDecoder(DecoderProtocol protocol, uint16_t address, bool longAddress) const
{
  auto it = std::find_if(begin(), end(),
    [protocol, address, longAddress](const auto& decoder)
    {
      return
        decoder->protocol.value() == protocol &&
        decoder->address.value() == address &&
        (protocol != DecoderProtocol::DCC || decoder->longAddress == longAddress);
    });
  if(it != end())
    return *it;
  return {};
}

void DecoderList::worldEvent(WorldState state, WorldEvent event)
{
  ObjectList<Decoder>::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(add, editable);
  Attributes::setEnabled(remove, editable);
}

bool DecoderList::isListedProperty(std::string_view name)
{
  return DecoderListTableModel::isListedProperty(name);
}
