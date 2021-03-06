/**
 * server/src/train/train.cpp
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

#include "train.hpp"
#include "../world/world.hpp"
#include "trainlisttablemodel.hpp"
#include "../core/attributes.hpp"
#include "../utils/displayname.hpp"

Train::Train(const std::weak_ptr<World>& world, std::string_view _id) :
  IdObject(world, _id),
  name{this, "name", "", PropertyFlags::ReadWrite | PropertyFlags::Store},
  lob{*this, "lob", 0, LengthUnit::MilliMeter, PropertyFlags::ReadOnly | PropertyFlags::Store},
  direction{this, "direction", Direction::Forward, PropertyFlags::ReadWrite | PropertyFlags::StoreState},
  speed{*this, "speed", 0, SpeedUnit::KiloMeterPerHour, PropertyFlags::ReadOnly | PropertyFlags::NoStore},
  speedMax{*this, "speed_max", 120, SpeedUnit::KiloMeterPerHour, PropertyFlags::ReadWrite | PropertyFlags::Store},
  throttleSpeed{*this, "throttle_speed", 0, SpeedUnit::KiloMeterPerHour, PropertyFlags::ReadWrite | PropertyFlags::StoreState},
  weight{*this, "weight", 0, WeightUnit::Ton, PropertyFlags::ReadOnly | PropertyFlags::Store},
  vehicles{this, "vehicles", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject},
  notes{this, "notes", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  vehicles.setValueInternal(std::make_shared<RailVehicleList>(*this, vehicles.name()));

  auto w = world.lock();
  const bool editable = w && contains(w->state.value(), WorldState::Edit);

  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);
  m_interfaceItems.add(lob);
  Attributes::addValues(direction, DirectionValues);
  Attributes::addObjectEditor(direction, false);
  m_interfaceItems.add(direction);
  Attributes::addObjectEditor(speed, false);
  m_interfaceItems.add(speed);
  Attributes::addEnabled(speedMax, editable);
  m_interfaceItems.add(speedMax);
  Attributes::addObjectEditor(throttleSpeed, false);
  m_interfaceItems.add(throttleSpeed);
  m_interfaceItems.add(weight);
  m_interfaceItems.add(vehicles);
  Attributes::addDisplayName(notes, DisplayName::Object::notes);
  m_interfaceItems.add(notes);
}

void Train::addToWorld()
{
  IdObject::addToWorld();

  if(auto world = m_world.lock())
    world->trains->addObject(shared_ptr<Train>());
}

void Train::worldEvent(WorldState state, WorldEvent event)
{
  IdObject::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  name.setAttributeEnabled(editable);
  speedMax.setAttributeEnabled(editable);
}
