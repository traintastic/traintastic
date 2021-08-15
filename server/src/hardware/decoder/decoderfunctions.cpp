/**
 * server/src/hardware/decoder/decoderfunctions.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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

#include "decoderfunctions.hpp"
#include "decoder.hpp"
#include "../../world/getworld.hpp"
#include "../../core/attributes.hpp"

DecoderFunctions::DecoderFunctions(Object& _parent, const std::string& parentPropertyName)
  : SubObject(_parent, parentPropertyName)
  , items{*this, "items", {}, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
  , add{*this, "add",
      [this]()
      {
        Decoder& decoder = static_cast<Decoder&>(this->parent());
        auto world = decoder.world().lock();
        if(!world)
          return std::shared_ptr<DecoderFunction>();
        uint8_t number = 0;
        if(!items.empty())
        {
          for(const auto& function : items)
            number = std::max(number, function->number.value());
          number++;
        }

        auto function = std::make_shared<DecoderFunction>(decoder, number);
        function->name = "F" + std::to_string(number);
        function->number = number;
        if(number == 0) // F0 is (almost) always the light function
          function->function = DecoderFunctionFunction::Light;
        items.appendInternal(function);

        return function;
      }}
  , remove{*this, "remove",
      [this](const std::shared_ptr<DecoderFunction>& function)
      {
        if(!function)
          return;
        function->destroy();
        items.removeInternal(function);
      }}
  , moveUp{*this, "move_up",
      [this](const std::shared_ptr<DecoderFunction>& function)
      {
        items.moveInternal(function, -1);
      }}
  , moveDown{*this, "move_down",
      [this](const std::shared_ptr<DecoderFunction>& function)
      {
        items.moveInternal(function, +1);
      }}
{
  auto world = getWorld(&_parent);
  const bool editable = world && contains(world->state.value(), WorldState::Edit);

  m_interfaceItems.add(items);
  Attributes::addEnabled(add, editable);
  m_interfaceItems.add(add);
  Attributes::addEnabled(remove, editable);
  m_interfaceItems.add(remove);
  Attributes::addEnabled(moveUp, editable);
  m_interfaceItems.add(moveUp);
  Attributes::addEnabled(moveDown, editable);
  m_interfaceItems.add(moveDown);
}

void DecoderFunctions::load(WorldLoader& loader, const nlohmann::json& data)
{
  nlohmann::json objects = data.value("items", nlohmann::json::array());
  if(data[items.name()].size() > 0)
  {
    Decoder& decoder = static_cast<Decoder&>(this->parent());
    std::vector<std::shared_ptr<DecoderFunction>> values;
    for(const auto& object : objects.items())
    {
      nlohmann::json number = object.value().value("number", nlohmann::json());
      if(number.is_number_unsigned())
        values.emplace_back(std::make_shared<DecoderFunction>(decoder, number.get<uint8_t>()));
      else
        break;
    }
    items.load(std::move(values));
  }
  SubObject::load(loader, data);
}

void DecoderFunctions::worldEvent(WorldState state, WorldEvent event)
{
  Object::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  add.setAttributeEnabled(editable);
  remove.setAttributeEnabled(editable);
  moveUp.setAttributeEnabled(editable);
  moveDown.setAttributeEnabled(editable);
}
