/**
 * server/src/board/tile/rail/sensorrailtile.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2021 Reinder Feenstra
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

#include "sensorrailtile.hpp"
#include "../../../world/world.hpp"
#include "../../../core/attributes.hpp"

SensorRailTile::SensorRailTile(const std::weak_ptr<World>& world, std::string_view _id) :
  StraightRailTile(world, _id, TileId::RailSensor),
  name{this, "name", id, PropertyFlags::ReadWrite | PropertyFlags::Store},
  input{this, "input", nullptr, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](const std::shared_ptr<Input>& value)
    {
      if(input)
        input->propertyChanged.disconnect(m_inputPropertyChanged);

      if(value)
      {
        m_inputPropertyChanged = value->propertyChanged.connect(std::bind(&SensorRailTile::inputPropertyChanged, this, std::placeholders::_1));
        inputPropertyChanged(input->value);
      }
      else
        state.setValueInternal(TriState::Undefined);

      return true;
    }},
  invert{this, "invert", false, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](bool)
    {
      if(input)
        inputPropertyChanged(input->value);
    }},
  state{this, "state", TriState::Undefined, PropertyFlags::ReadOnly | PropertyFlags::StoreState}
{
  auto w = world.lock();
  const bool editable = w && contains(w->state.value(), WorldState::Edit);

  Attributes::addEnabled(name, editable);
  Attributes::addDisplayName(name, "object:name");
  m_interfaceItems.add(name);
  Attributes::addEnabled(input, editable);
  Attributes::addObjectList(input, w->inputs);
  m_interfaceItems.add(input);
  Attributes::addEnabled(invert, editable);
  m_interfaceItems.add(invert);
  Attributes::addValues(state, TriStateValues);
  m_interfaceItems.add(state);
}

void SensorRailTile::loaded()
{
  StraightRailTile::loaded();

  if(input)
    m_inputPropertyChanged = input->propertyChanged.connect(std::bind(&SensorRailTile::inputPropertyChanged, this, std::placeholders::_1));
}

void SensorRailTile::worldEvent(WorldState state, WorldEvent event)
{
  StraightRailTile::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(name, editable);
  Attributes::setEnabled(input, editable);
  Attributes::setEnabled(invert, editable);
}

void SensorRailTile::inputPropertyChanged(BaseProperty& property)
{
  assert(input);
  if(&property == static_cast<BaseProperty*>(&input->value))
    state.setValueInternal(input->value.value() ^ invert.value());
}
