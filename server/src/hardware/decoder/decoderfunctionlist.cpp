/**
 * server/src/hardware/decoder/decoderfunctionlist.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

#include "decoderfunctionlist.hpp"
#include "decoderfunctionlisttablemodel.hpp"
#include "decoder.hpp"
#include "../../world/getworld.hpp"
#include "../../core/attributes.hpp"

DecoderFunctionList::DecoderFunctionList(Object& _parent, const std::string& parentPropertyName) :
  ObjectList<DecoderFunction>(_parent, parentPropertyName),
  add{*this, "add",
    [this]()
    {
      Decoder& decoder = static_cast<Decoder&>(this->parent());
      auto world = decoder.world().lock();
      if(!world)
        return std::shared_ptr<DecoderFunction>();
      uint8_t number = 0;
      if(!m_items.empty())
      {
        for(const auto& function : m_items)
          number = std::max(number, function->number.value());
        number++;
      }

      std::string id = decoder.id.value() + "_f" + std::to_string(number);
      if(world->isObject(id))
        id = world->getUniqueId(id);

      auto function = DecoderFunction::create(decoder, id);
      function->name = "F" + std::to_string(number);
      function->number = number;
      addObject(function);

      return function;
    }}
{
  auto world = getWorld(&_parent);
  const bool editable = world && contains(world->state.value(), WorldState::Edit);

  Attributes::addEnabled(add, editable);
  m_interfaceItems.add(add);
}

TableModelPtr DecoderFunctionList::getModel()
{
  return std::make_shared<DecoderFunctionListTableModel>(*this);
}

void DecoderFunctionList::worldEvent(WorldState state, WorldEvent event)
{
  ObjectList<DecoderFunction>::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  add.setAttributeEnabled(editable);
}

bool DecoderFunctionList::isListedProperty(const std::string& name)
{
  return DecoderFunctionListTableModel::isListedProperty(name);
}
