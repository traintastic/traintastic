/**
 * server/src/board/tile/misc/switchtile.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#include "switchtile.hpp"
#include "../../../world/world.hpp"
#include "../../../core/objectproperty.tpp"
#include "../../../core/method.tpp"
#include "../../../core/attributes.hpp"
#include "../../../hardware/output/map/switchoutputmap.hpp"
#include "../../../utils/displayname.hpp"

CREATE_IMPL(SwitchTile)

static constexpr std::array<bool, 2> valueAliasKeys{{false, true}};
static const std::array<std::string, 2> valueAliasValues{{"$output_map_item.switch.key:off$", "$output_map_item.switch.key:on$"}};

SwitchTile::SwitchTile(World& world, std::string_view _id)
  : Tile(world, _id, TileId::Switch)
  , name{this, "name", id, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly}
  , colorOn{this, "color_on", Color::Yellow, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly}
  , colorOff{this, "color_off", Color::Gray, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly}
  , value{this, "value", false, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::ScriptReadOnly}
  , outputMap{this, "output_map", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject | PropertyFlags::NoScript}
  , toggle{*this, "toggle", MethodFlags::ScriptCallable,
      [this]()
      {
        value = !value;
      }}
  , setValue(*this, "set_value", MethodFlags::ScriptCallable,
      [this](bool newValue)
      {
        if(value != newValue)
        {
          (*outputMap)[newValue]->execute();
          value.setValueInternal(newValue);
          fireEvent(onValueChanged, shared_ptr<SwitchTile>(), newValue);
        }
      })
  , onValueChanged{*this, "on_value_changed", EventFlags::Scriptable}
{
  outputMap.setValueInternal(std::make_shared<SwitchOutputMap>(*this, outputMap.name()));

  const bool editable = contains(m_world.state.value(), WorldState::Edit);

  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);

  Attributes::addEnabled(colorOn, editable);
  Attributes::addValues(colorOn, colorValuesWithoutNone);
  m_interfaceItems.add(colorOn);

  Attributes::addEnabled(colorOff, editable);
  Attributes::addValues(colorOff, colorValuesWithoutNone);
  m_interfaceItems.add(colorOff);

  Attributes::addObjectEditor(value, false);
  Attributes::addAliases(value, tcb::span<const bool>(valueAliasKeys), tcb::span<const std::string>(valueAliasValues));
  m_interfaceItems.add(value);

  Attributes::addDisplayName(outputMap, DisplayName::BoardTile::outputMap);
  m_interfaceItems.add(outputMap);

  Attributes::addObjectEditor(toggle, false);
  m_interfaceItems.add(toggle);

  Attributes::addObjectEditor(setValue, false);
  m_interfaceItems.add(setValue);

  m_interfaceItems.add(onValueChanged);
}

void SwitchTile::destroying()
{
  outputMap->parentObject.setValueInternal(nullptr);
  Tile::destroying();
}

void SwitchTile::addToWorld()
{
  outputMap->parentObject.setValueInternal(shared_from_this());
  Tile::addToWorld();
}

void SwitchTile::worldEvent(WorldState worldState, WorldEvent worldEvent)
{
  Tile::worldEvent(worldState, worldEvent);

  const bool editable = contains(worldState, WorldState::Edit);

  Attributes::setEnabled(name, editable);
  Attributes::setEnabled(colorOn, editable);
  Attributes::setEnabled(colorOff, editable);
}
