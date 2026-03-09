/**
 * server/src/board/tile/rail/sensorrailtile.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2025 Reinder Feenstra
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
#include "../../../core/method.tpp"
#include "../../../core/objectproperty.tpp"
#include "../../../hardware/input/inputcontroller.hpp"
#include "../../../utils/sensor.hpp"
#include "../../../utils/category.hpp"
#include "../../../utils/displayname.hpp"

SensorRailTile::SensorRailTile(World& world, std::string_view _id) :
  StraightRailTile(world, _id, TileId::RailSensor),
  InputConsumer(static_cast<Object&>(*this), world),
  name{this, "name", id, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly},
  type{this, "type", SensorType::OccupancyDetector, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](SensorType /*value*/)
    {
      if(input() && input()->value != TriState::Undefined)
      {
        inputValueChanged(input()->value == TriState::True, input());
      }
    }},
  invert{this, "invert", false, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](bool /*value*/)
    {
      if(input() && input()->value != TriState::Undefined)
      {
        inputValueChanged(input()->value == TriState::True, input());
      }
    }},
  state{this, "state", SensorState::Unknown, PropertyFlags::ReadOnly | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly}
  , simulateTrigger{*this, "simulate_trigger",
      [this]()
      {
        if(input()) /*[[likely]]*/
        {
          input()->interface->inputSimulateChange(input()->channel, input()->address, SimulateInputAction::Toggle);
        }
      }}
  , onStateChanged{*this, "on_state_changed", EventFlags::Scriptable}
{
  const bool editable = contains(m_world.state.value(), WorldState::Edit);

  Attributes::addEnabled(name, editable);
  Attributes::addDisplayName(name, DisplayName::Object::name);
  m_interfaceItems.add(name);

  Attributes::addCategory(type, Category::general);
  Attributes::addEnabled(type, editable);
  Attributes::addValues(type, sensorTypeValues);
  m_interfaceItems.add(type);

  InputConsumer::addInterfaceItems(m_interfaceItems);

  Attributes::addCategory(invert, Category::input);
  Attributes::addEnabled(invert, editable);
  m_interfaceItems.add(invert);

  Attributes::addObjectEditor(state, false);
  Attributes::addValues(state, sensorStateValues);
  m_interfaceItems.add(state);

  Attributes::addEnabled(simulateTrigger, false);
  Attributes::addObjectEditor(simulateTrigger, false);
  m_interfaceItems.add(simulateTrigger);

  m_interfaceItems.add(onStateChanged);
}

//! \todo Remove in v0.4
void SensorRailTile::load(WorldLoader& loader, const nlohmann::json& data)
{
  if(data["type"] == "occupy_detector")
  {
    nlohmann::json dataCopy = data;
    dataCopy["type"] = "occupancy_detector";
    StraightRailTile::load(loader, dataCopy);
  }
  else
  {
    StraightRailTile::load(loader, data);
  }
}

void SensorRailTile::loaded()
{
  StraightRailTile::loaded();
  InputConsumer::loaded();
  updateSimulateTriggerEnabled();
}

void SensorRailTile::destroying()
{
  StraightRailTile::destroying();
}

void SensorRailTile::worldEvent(WorldState worldState, WorldEvent worldEvent)
{
  StraightRailTile::worldEvent(worldState, worldEvent);
  InputConsumer::worldEvent(worldState, worldEvent);

  const bool editable = contains(worldState, WorldState::Edit);

  Attributes::setEnabled(name, editable);
  Attributes::setEnabled(type, editable);
  Attributes::setEnabled(invert, editable);
  updateSimulateTriggerEnabled();
}

void SensorRailTile::inputValueChanged(bool value, const std::shared_ptr<Input>& /*input*/)
{
  const auto newState = toSensorState(type, toTriState(value != invert.value()));
  if(state != newState)
  {
    state.setValueInternal(newState);
    fireEvent(onStateChanged, newState, shared_ptr<SensorRailTile>());
  }
}

void SensorRailTile::updateSimulateTriggerEnabled()
{
  Attributes::setEnabled(simulateTrigger, contains(m_world.state, WorldState::Online | WorldState::Simulation) /*&& input*/);
}
