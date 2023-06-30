/**
 * server/src/vehicle/rail/freightwagon.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2023 Reinder Feenstra
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

#include "freightwagon.hpp"
#include "../../world/world.hpp"
#include "../../core/attributes.hpp"
#include "../../utils/category.hpp"
#include "../../utils/displayname.hpp"

FreightWagon::FreightWagon(World& world, std::string_view _id)
  : UnpoweredRailVehicle(world, _id)
  , cargoLoaded{*this, "cargo_loaded", 0, RatioUnit::Percent, PropertyFlags::ReadWrite | PropertyFlags::NoStore,
    [this](double value, RatioUnit unit)
    {
      const double ratio = std::clamp(convertUnit(value, unit, RatioUnit::Ratio), 0., 1.);
      cargoWeight.setValue(ratio * cargoCapacity.getValue(cargoWeight.unit()));
    }}
  , cargoWeight{*this, "cargo_weight", 0, WeightUnit::Ton, PropertyFlags::ReadWrite | PropertyFlags::StoreState,
    [this](double /*value*/, WeightUnit /*unit*/)
    {
      updateCargoLoaded();
      updateTotalWeight();
    }}
  , cargoCapacity{*this, "cargo_capacity", 0, WeightUnit::Ton, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](double value, WeightUnit unit)
    {
      Attributes::setMax(cargoWeight, value, unit);
      if(cargoWeight.getValue(unit) > value)
        cargoWeight.setValue(convertUnit(value, unit, cargoWeight.unit()));
      updateCargoLoaded();
    }}
{
  const bool editable = contains(m_world.state.value(), WorldState::Edit);

  Attributes::addCategory(cargoLoaded, Category::cargo);
  Attributes::addDisplayName(cargoLoaded, DisplayName::Vehicle::Rail::cargoLoaded);
  Attributes::addMinMax(cargoLoaded, 0., 100., RatioUnit::Percent);
  m_interfaceItems.insertBefore(cargoLoaded, totalWeight);

  Attributes::addCategory(cargoWeight, Category::cargo);
  Attributes::addMinMax(cargoWeight, 0., 0., WeightUnit::Ton);
  m_interfaceItems.insertBefore(cargoWeight, totalWeight);

  Attributes::addCategory(cargoCapacity, Category::cargo);
  Attributes::addDisplayName(cargoCapacity, DisplayName::Vehicle::Rail::cargoCapacity);
  Attributes::addEnabled(cargoCapacity, editable);
  m_interfaceItems.insertBefore(cargoCapacity, totalWeight);
}

double FreightWagon::calcTotalWeight(WeightUnit unit) const
{
  return RailVehicle::calcTotalWeight(unit) + cargoWeight.getValue(unit);
}

void FreightWagon::loaded()
{
  UnpoweredRailVehicle::loaded();
  updateCargoLoaded();
  Attributes::setMax(cargoWeight, cargoCapacity.value(), cargoCapacity.unit());
}

void FreightWagon::worldEvent(WorldState state, WorldEvent event)
{
  UnpoweredRailVehicle::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(cargoCapacity, editable);
}

void FreightWagon::updateCargoLoaded()
{
  const double capacity = cargoCapacity.getValue(WeightUnit::Ton);
  if(capacity > 0)
    cargoLoaded.setValueInternal(convertUnit(cargoWeight.getValue(WeightUnit::Ton) / capacity, RatioUnit::Ratio, cargoLoaded.unit()));
  else
    cargoLoaded.setValueInternal(0);
}
