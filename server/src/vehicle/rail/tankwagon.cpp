/**
 * server/src/vehicle/rail/tankwagon.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#include "tankwagon.hpp"
#include "../../world/world.hpp"
#include "../../core/attributes.hpp"
#include "../../utils/category.hpp"
#include "../../utils/displayname.hpp"

TankWagon::TankWagon(World& world, std::string_view _id)
  : UnpoweredRailVehicle(world, _id)
  , cargoLoaded{*this, "cargo_loaded", 0, RatioUnit::Percent, PropertyFlags::ReadWrite | PropertyFlags::NoStore,
    [this](double value, RatioUnit unit)
    {
      const double ratio = std::clamp(convertUnit(value, unit, RatioUnit::Ratio), 0., 1.);
      cargoVolume.setValue(ratio * cargoCapacity.getValue(cargoVolume.unit()));
    }}
  , cargoVolume{*this, "cargo_volume", 0, VolumeUnit::CubicMeter, PropertyFlags::ReadWrite | PropertyFlags::StoreState,
    [this](double /*value*/, VolumeUnit /*unit*/)
    {
      updateCargoLoaded();
      updateTotalWeight();
    }}
  , cargoCapacity{*this, "cargo_capacity", 0, VolumeUnit::CubicMeter, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](double value, VolumeUnit unit)
    {
      Attributes::setMax(cargoVolume, value, unit);
      if(cargoVolume.getValue(unit) > value)
        cargoVolume.setValue(convertUnit(value, unit, cargoVolume.unit()));
      updateCargoLoaded();
    }}
{
  const bool editable = contains(m_world.state.value(), WorldState::Edit);

  Attributes::addCategory(cargoLoaded, Category::cargo);
  Attributes::addDisplayName(cargoLoaded, DisplayName::Vehicle::Rail::cargoLoaded);
  Attributes::addMinMax(cargoLoaded, 0., 100., RatioUnit::Percent);
  m_interfaceItems.insertBefore(cargoLoaded, totalWeight);

  Attributes::addCategory(cargoVolume, Category::cargo);
  Attributes::addMinMax(cargoVolume, 0., 0., VolumeUnit::CubicMeter);
  m_interfaceItems.insertBefore(cargoVolume, totalWeight);

  Attributes::addCategory(cargoCapacity, Category::cargo);
  Attributes::addDisplayName(cargoCapacity, DisplayName::Vehicle::Rail::cargoCapacity);
  Attributes::addEnabled(cargoCapacity, editable);
  m_interfaceItems.insertBefore(cargoCapacity, totalWeight);
}

double TankWagon::calcTotalWeight(WeightUnit unit) const
{
  return RailVehicle::calcTotalWeight(unit) + convertUnit(cargoVolume.getValue(VolumeUnit::CubicMeter) * cargoDensity, WeightUnit::KiloGram, unit);
}

void TankWagon::loaded()
{
  UnpoweredRailVehicle::loaded();
  updateCargoLoaded();
  Attributes::setMax(cargoVolume, cargoCapacity.value(), cargoCapacity.unit());
}

void TankWagon::worldEvent(WorldState state, WorldEvent event)
{
  UnpoweredRailVehicle::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(cargoCapacity, editable);
}

void TankWagon::updateCargoLoaded()
{
  const double capacity = cargoCapacity.getValue(VolumeUnit::CubicMeter);
  if(capacity > 0)
    cargoLoaded.setValueInternal(convertUnit(cargoVolume.getValue(VolumeUnit::CubicMeter) / capacity, RatioUnit::Ratio, cargoLoaded.unit()));
  else
    cargoLoaded.setValueInternal(0);
}
