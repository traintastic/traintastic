/**
 * server/src/zone/zone.cpp
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

#include "zone.hpp"
#include "zoneblocklist.hpp"
#include "zonelist.hpp"
#include "zonelisttablemodel.hpp"
#include "../core/attributes.hpp"
#include "../core/objectproperty.tpp"
#include "../core/objectvectorproperty.tpp"
#include "../train/train.hpp"
#include "../utils/displayname.hpp"
#include "../world/world.hpp"

CREATE_IMPL(Zone)

Zone::Zone(World& world, std::string_view id_)
  : IdObject(world, id_)
  , name{this, "name", id, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly}
  , mute{this, "mute", false, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly,
      [this](bool /*value*/)
      {
        for(auto& status : trains)
        {
          status->train->updateMute();
        }
      }}
  , noSmoke{this, "no_smoke", false, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly,
      [this](bool /*value*/)
      {
        for(auto& status : trains)
        {
          status->train->updateNoSmoke();
        }
      }}
  , speedLimit{*this, "speed_limit", SpeedLimitProperty::noLimitValue, SpeedUnit::KiloMeterPerHour, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly,
      [this](double /*value*/, SpeedUnit /*unit*/)
      {
        for(auto& status : trains)
        {
          status->train->updateSpeedLimit();
        }
      }}
  , blocks{this, "blocks", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
  , trains{*this, "trains", {}, PropertyFlags::ReadOnly | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly}
  , onTrainAssigned{*this, "on_train_assigned", EventFlags::Scriptable}
  , onTrainEntering{*this, "on_train_entering", EventFlags::Scriptable}
  , onTrainEntered{*this, "on_train_entered", EventFlags::Scriptable}
  , onTrainLeaving{*this, "on_train_leaving", EventFlags::Scriptable}
  , onTrainLeft{*this, "on_train_left", EventFlags::Scriptable}
  , onTrainRemoved{*this, "on_train_removed", EventFlags::Scriptable}
{
  blocks.setValueInternal(std::make_shared<ZoneBlockList>(*this, blocks.name()));

  const bool editable = contains(world.state.value(), WorldState::Edit);

  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);

  Attributes::addEnabled(mute, editable);
  m_interfaceItems.add(mute);

  Attributes::addEnabled(noSmoke, editable);
  m_interfaceItems.add(noSmoke);

  Attributes::addEnabled(speedLimit, editable);
  m_interfaceItems.add(speedLimit);

  m_interfaceItems.add(blocks);

  Attributes::addObjectEditor(trains, false);
  m_interfaceItems.add(trains);

  m_interfaceItems.add(onTrainAssigned);
  m_interfaceItems.add(onTrainEntering);
  m_interfaceItems.add(onTrainEntered);
  m_interfaceItems.add(onTrainLeaving);
  m_interfaceItems.add(onTrainLeft);
  m_interfaceItems.add(onTrainRemoved);
}

std::shared_ptr<TrainZoneStatus> Zone::getTrainZoneStatus(const std::shared_ptr<Train>& train)
{
  auto it = std::find_if(trains.begin(), trains.end(),
    [&train](const auto& status)
    {
      return status->train.value() == train;
    });

  return (it != trains.end()) ? *it : std::shared_ptr<TrainZoneStatus>{};
}

void Zone::worldEvent(WorldState worldState, WorldEvent worldEvent)
{
  IdObject::worldEvent(worldState, worldEvent);

  const bool editable = contains(worldState, WorldState::Edit);

  Attributes::setEnabled({name, mute, noSmoke, speedLimit}, editable);
}

void Zone::addToWorld()
{
  IdObject::addToWorld();

  m_world.zones->addObject(shared_ptr<Zone>());
}

void Zone::fireTrainAssigned(const std::shared_ptr<Train>& train)
{
  fireEvent(onTrainAssigned, train, shared_ptr<Zone>());
}

void Zone::fireTrainEntering(const std::shared_ptr<Train>& train)
{
  fireEvent(onTrainEntering, train, shared_ptr<Zone>());
}

void Zone::fireTrainEntered(const std::shared_ptr<Train>& train)
{
  fireEvent(onTrainEntered, train, shared_ptr<Zone>());
}

void Zone::fireTrainLeaving(const std::shared_ptr<Train>& train)
{
  fireEvent(onTrainLeaving, train, shared_ptr<Zone>());
}

void Zone::fireTrainLeft(const std::shared_ptr<Train>& train)
{
  fireEvent(onTrainLeft, train, shared_ptr<Zone>());
}

void Zone::fireTrainRemoved(const std::shared_ptr<Train>& train)
{
  fireEvent(onTrainRemoved, train, shared_ptr<Zone>());
}
