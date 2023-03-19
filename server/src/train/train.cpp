/**
 * server/src/train/train.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021,2023 Reinder Feenstra
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
#include "../vehicle/rail/poweredrailvehicle.hpp"
#include "../utils/almostzero.hpp"
#include "../utils/displayname.hpp"

static inline bool isPowered(const RailVehicle& vehicle)
{
  return dynamic_cast<const PoweredRailVehicle*>(&vehicle);
}

Train::Train(World& world, std::string_view _id) :
  IdObject(world, _id),
  name{this, "name", "", PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly},
  lob{*this, "lob", 0, LengthUnit::MilliMeter, PropertyFlags::ReadWrite | PropertyFlags::Store},
  overrideLength{this, "override_length", false, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](bool value)
    {
      Attributes::setEnabled(lob, value);
      if(!value)
        updateLength();
    }},
  direction{this, "direction", Direction::Forward, PropertyFlags::ReadWrite | PropertyFlags::StoreState,
    [this](Direction value)
    {
      for(const auto& vehicle : m_poweredVehicles)
        vehicle->setDirection(value);
      updateEnabled();
    }},
  speed{*this, "speed", 0, SpeedUnit::KiloMeterPerHour, PropertyFlags::ReadOnly | PropertyFlags::NoStore},
  speedMax{*this, "speed_max", 0, SpeedUnit::KiloMeterPerHour, PropertyFlags::ReadWrite | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly},
  throttleSpeed{*this, "throttle_speed", 0, SpeedUnit::KiloMeterPerHour, PropertyFlags::ReadWrite | PropertyFlags::StoreState,
    [this](double value, SpeedUnit unit)
    {
      setSpeed(convertUnit(value, unit, SpeedUnit::KiloMeterPerHour)); // just set speed for now, no train fysics yet
    }},
  weight{*this, "weight", 0, WeightUnit::Ton, PropertyFlags::ReadWrite | PropertyFlags::Store},
  overrideWeight{this, "override_weight", false, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](bool value)
    {
      Attributes::setEnabled(weight, value);
      if(!value)
        updateWeight();
    }},
  vehicles{this, "vehicles", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject},
  powered{this, "powered", false, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly},
  notes{this, "notes", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  vehicles.setValueInternal(std::make_shared<TrainVehicleList>(*this, vehicles.name()));

  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, false);
  m_interfaceItems.add(name);
  Attributes::addEnabled(lob, overrideLength);
  m_interfaceItems.add(lob);
  m_interfaceItems.add(overrideLength);
  Attributes::addEnabled(direction, false);
  Attributes::addValues(direction, DirectionValues);
  Attributes::addObjectEditor(direction, false);
  m_interfaceItems.add(direction);
  Attributes::addObjectEditor(speed, false);
  Attributes::addMinMax(speed, 0., 0., SpeedUnit::KiloMeterPerHour);
  m_interfaceItems.add(speed);
  m_interfaceItems.add(speedMax);
  Attributes::addMinMax(throttleSpeed, 0., 0., SpeedUnit::KiloMeterPerHour);
  Attributes::addEnabled(throttleSpeed, false);
  Attributes::addObjectEditor(throttleSpeed, false);
  m_interfaceItems.add(throttleSpeed);
  Attributes::addEnabled(weight, overrideWeight);
  m_interfaceItems.add(weight);
  m_interfaceItems.add(overrideWeight);
  m_interfaceItems.add(vehicles);
  Attributes::addObjectEditor(powered, false);
  m_interfaceItems.add(powered);
  Attributes::addDisplayName(notes, DisplayName::Object::notes);
  m_interfaceItems.add(notes);

  updateEnabled();
}

void Train::addToWorld()
{
  IdObject::addToWorld();
  m_world.trains->addObject(shared_ptr<Train>());
}

void Train::destroying()
{
  m_world.trains->removeObject(shared_ptr<Train>());
  IdObject::destroying();
}

void Train::loaded()
{
  IdObject::loaded();

  Attributes::setEnabled(lob, overrideLength);
  Attributes::setEnabled(weight, overrideWeight);

  vehiclesChanged();
}

void Train::worldEvent(WorldState state, WorldEvent event)
{
  IdObject::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(name, editable);
}

void Train::setSpeed(const double kmph)
{
  for(const auto& vehicle : m_poweredVehicles)
    vehicle->setSpeed(kmph);
  speed.setValueInternal(convertUnit(kmph, SpeedUnit::KiloMeterPerHour, speed.unit()));
  updateEnabled();
}

void Train::vehiclesChanged()
{
  updateLength();
  updateWeight();
  updatePowered();
  updateSpeedMax();
}

void Train::updateLength()
{
  if(overrideLength)
    return;

  double mm = 0;
  for(const auto& vehicle : *vehicles)
    mm += vehicle->lob.getValue(LengthUnit::MilliMeter);
  lob.setValueInternal(convertUnit(mm, LengthUnit::MilliMeter, lob.unit()));
}

void Train::updateWeight()
{
  if(overrideWeight)
    return;

  double ton = 0;
  for(const auto& vehicle : *vehicles)
    ton += vehicle->totalWeight.getValue(WeightUnit::Ton);
  weight.setValueInternal(convertUnit(ton, WeightUnit::Ton, weight.unit()));
}

void Train::updatePowered()
{
  m_poweredVehicles.clear();
  for(const auto& vehicle : *vehicles)
    if(auto poweredVehicle = std::dynamic_pointer_cast<PoweredRailVehicle>(vehicle))
      m_poweredVehicles.emplace_back(poweredVehicle);
  powered.setValueInternal(!m_poweredVehicles.empty());
}

void Train::updateSpeedMax()
{
  if(!vehicles->empty() && powered)
  {
    const auto itEnd = vehicles->end();
    auto it = vehicles->begin();
    double kmph = (*it)->speedMax.getValue(SpeedUnit::KiloMeterPerHour);
    for(; it != itEnd; ++it)
    {
      const double v = (*it)->speedMax.getValue(SpeedUnit::KiloMeterPerHour);
      if((v > 0 || isPowered(**it)) && v < kmph)
        kmph = v;
    }
    speedMax.setValueInternal(convertUnit(kmph, SpeedUnit::KiloMeterPerHour, speedMax.unit()));
  }
  else
    speedMax.setValueInternal(0);

  speed.setUnit(speedMax.unit());
  Attributes::setMax(speed, speedMax.value(), speedMax.unit());
  throttleSpeed.setUnit(speedMax.unit());
  Attributes::setMax(throttleSpeed, speedMax.value(), speedMax.unit());
}

void Train::updateEnabled()
{
  const bool stopped = almostZero(speed.value()) && almostZero(throttleSpeed.value());

  Attributes::setEnabled(name, stopped);
  Attributes::setEnabled(direction, stopped && powered);
  Attributes::setEnabled(throttleSpeed, powered);
  Attributes::setEnabled(vehicles->add, stopped);
  Attributes::setEnabled(vehicles->remove, stopped);
  Attributes::setEnabled(vehicles->move, stopped);
  Attributes::setEnabled(vehicles->reverse, stopped);
}
