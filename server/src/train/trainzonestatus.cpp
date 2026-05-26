/**
 * server/src/train/trainzonestatus.cpp
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

#include "trainzonestatus.hpp"
#include "train.hpp"
#include "../core/objectproperty.tpp"
#include "../core/attributes.hpp"
#include "../world/world.hpp"
#include "../zone/zone.hpp"

std::shared_ptr<TrainZoneStatus> TrainZoneStatus::create(Zone& zone_, Train& train_, ZoneTrainState state_, std::string_view id)
{
  World& world = zone_.world();
  auto p = std::make_shared<TrainZoneStatus>(id.empty() ? world.getUniqueId(TrainZoneStatus::classId) : std::string{id});
  p->zone.setValueInternal(zone_.shared_ptr<Zone>());
  p->train.setValueInternal(train_.shared_ptr<Train>());
  p->state.setValueInternal(state_);
  TrainZoneStatus::addToWorld(world, *p);
  return p;
}

TrainZoneStatus::TrainZoneStatus(std::string id)
  : StateObject(std::move(id))
  , zone{this, "zone", nullptr, PropertyFlags::ReadOnly | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly}
  , train{this, "train", nullptr, PropertyFlags::ReadOnly | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly}
  , state{this, "state", ZoneTrainState::Unknown, PropertyFlags::ReadOnly | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly}
{
  m_interfaceItems.add(zone);

  m_interfaceItems.add(train);

  Attributes::addValues(state, zoneTrainStateValues);
  m_interfaceItems.add(state);
}

void TrainZoneStatus::destroying()
{
  auto self = shared_ptr<TrainZoneStatus>();
  if(zone)
    zone->trains.removeInternal(self);
  if(train)
    train->zones.removeInternal(self);
  removeFromWorld(zone->world(), *this);
  StateObject::destroying();
}
