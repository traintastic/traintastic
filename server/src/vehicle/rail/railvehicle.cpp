/**
 * server/src/vehicle/rail/railvehicle.cpp
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

#include "railvehicle.hpp"
#include "railvehiclelisttablemodel.hpp"
#include "../../world/world.hpp"
#include "../../core/attributes.hpp"

RailVehicle::RailVehicle(const std::weak_ptr<World>& world, std::string_view _id) :
  Vehicle(world, _id),
  decoder{this, "decoder", nullptr, PropertyFlags::ReadWrite | PropertyFlags::Store},
  lob{*this, "lob", 0, LengthUnit::MilliMeter, PropertyFlags::ReadWrite | PropertyFlags::Store},
  speedMax{*this, "speed_max", 0, SpeedUnit::KiloMeterPerHour, PropertyFlags::ReadWrite | PropertyFlags::Store},
  weight{*this, "weight", 0, WeightUnit::Ton, PropertyFlags::ReadWrite | PropertyFlags::Store, [this](double, WeightUnit){ updateTotalWeight(); }},
  totalWeight{*this, "total_weight", 0, WeightUnit::Ton, PropertyFlags::ReadOnly | PropertyFlags::NoStore}
{
  auto w = world.lock();
  const bool editable = w && contains(w->state.value(), WorldState::Edit);

  Attributes::addEnabled(decoder, editable);
  m_interfaceItems.insertBefore(decoder, notes);
  Attributes::addEnabled(lob, editable);
  m_interfaceItems.insertBefore(lob, notes);
  Attributes::addEnabled(speedMax, editable);
  m_interfaceItems.insertBefore(speedMax, notes);
  Attributes::addEnabled(weight, editable);
  m_interfaceItems.insertBefore(weight, notes);
  m_interfaceItems.insertBefore(totalWeight, notes);
}

void RailVehicle::addToWorld()
{
  Vehicle::addToWorld();

  if(auto world = m_world.lock())
    world->railVehicles->addObject(shared_ptr<RailVehicle>());
}

void RailVehicle::worldEvent(WorldState state, WorldEvent event)
{
  Vehicle::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  decoder.setAttributeEnabled(editable);
  lob.setAttributeEnabled(editable);
  speedMax.setAttributeEnabled(editable);
  weight.setAttributeEnabled(editable);
}

double RailVehicle::calcTotalWeight(WeightUnit unit) const
{
  return weight.getValue(unit);
}

void RailVehicle::updateTotalWeight()
{
  totalWeight.setValueInternal(calcTotalWeight(totalWeight.unit()));
}
