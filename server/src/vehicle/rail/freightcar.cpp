/**
 * server/src/vehicle/rail/freightcar.cpp
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

#include "freightcar.hpp"
#include "../../world/world.hpp"
#include "../../core/attributes.hpp"

FreightCar::FreightCar(const std::weak_ptr<World>& world, std::string_view _id) :
  RailVehicle(world, _id),
  cargoWeight{*this, "cargo_weight", 0, WeightUnit::Ton, PropertyFlags::ReadWrite | PropertyFlags::Store, [this](double, WeightUnit){ updateTotalWeight(); }}
{
  auto w = world.lock();
  const bool editable = w && contains(w->state.value(), WorldState::Edit);

  Attributes::addEnabled(cargoWeight, editable);
  m_interfaceItems.insertBefore(cargoWeight, totalWeight);
}

double FreightCar::calcTotalWeight(WeightUnit unit) const
{
  return RailVehicle::calcTotalWeight(unit) + cargoWeight.getValue(unit);
}

void FreightCar::worldEvent(WorldState state, WorldEvent event)
{
  RailVehicle::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  cargoWeight.setAttributeEnabled(editable);
}
