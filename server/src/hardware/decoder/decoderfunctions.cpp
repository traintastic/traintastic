/**
 * server/src/hardware/decoder/decoderfunctions.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021,2023 Reinder Feenstra
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
#include "../../core/method.tpp"
#include "../../core/objectvectorproperty.tpp"
#include "../../utils/displayname.hpp"

DecoderFunctions::DecoderFunctions(Object& _parent, std::string_view parentPropertyName)
  : SubObject(_parent, parentPropertyName)
  , items{*this, "items", {}, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
  , create{*this, "create",
      [this]() -> void
      {
        Decoder& decoder = static_cast<Decoder&>(this->parent());
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
      }}
  , delete_{*this, "delete",
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
  const bool editable = contains(getWorld(parent()).state.value(), WorldState::Edit);

  m_interfaceItems.add(items);

  Attributes::addDisplayName(create, DisplayName::List::create);
  Attributes::addEnabled(create, editable);
  m_interfaceItems.add(create);

  Attributes::addDisplayName(delete_, DisplayName::List::delete_);
  Attributes::addEnabled(delete_, editable);
  m_interfaceItems.add(delete_);

  Attributes::addDisplayName(moveUp, DisplayName::List::moveUp);
  Attributes::addEnabled(moveUp, editable);
  m_interfaceItems.add(moveUp);

  Attributes::addDisplayName(moveDown, DisplayName::List::moveDown);
  Attributes::addEnabled(moveDown, editable);
  m_interfaceItems.add(moveDown);
}

void DecoderFunctions::load(WorldLoader& loader, const nlohmann::json& data)
{
  nlohmann::json objects = data.value("items", nlohmann::json::array());
  if(!objects.empty())
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

  Attributes::setEnabled(create, editable);
  Attributes::setEnabled(delete_, editable);
  Attributes::setEnabled(moveUp, editable);
  Attributes::setEnabled(moveDown, editable);
}
