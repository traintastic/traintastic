/**
 * server/src/vehicle/rail/railvehicle.cpp
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

#include "railvehicle.hpp"
#include "railvehiclelist.hpp"
#include "railvehiclelisttablemodel.hpp"
#include "../../hardware/decoder/decoder.hpp"
#include "../../world/world.hpp"
#include "../../core/attributes.hpp"
#include "../../core/objectproperty.tpp"
#include "../../utils/displayname.hpp"

RailVehicle::RailVehicle(World& world, std::string_view _id) :
  Vehicle(world, _id),
  decoder{this, "decoder", nullptr, PropertyFlags::ReadWrite | PropertyFlags::Store},
  lob{*this, "lob", 0, LengthUnit::MilliMeter, PropertyFlags::ReadWrite | PropertyFlags::Store},
  speedMax{*this, "speed_max", 0, SpeedUnit::KiloMeterPerHour, PropertyFlags::ReadWrite | PropertyFlags::Store},
  weight{*this, "weight", 0, WeightUnit::Ton, PropertyFlags::ReadWrite | PropertyFlags::Store, [this](double /*value*/, WeightUnit /*unit*/){ updateTotalWeight(); }},
  totalWeight{*this, "total_weight", 0, WeightUnit::Ton, PropertyFlags::ReadOnly | PropertyFlags::NoStore},
  activeTrain{this, "active_train", nullptr, PropertyFlags::ReadOnly | PropertyFlags::ScriptReadOnly | PropertyFlags::StoreState}
{
  const bool editable = contains(m_world.state.value(), WorldState::Edit);

  Attributes::addDisplayName(decoder, DisplayName::Vehicle::Rail::decoder);
  Attributes::addEnabled(decoder, editable);
  Attributes::addObjectList(decoder, m_world.decoders);
  m_interfaceItems.insertBefore(decoder, notes);

  Attributes::addDisplayName(lob, DisplayName::Vehicle::Rail::lob);
  Attributes::addEnabled(lob, editable);
  m_interfaceItems.insertBefore(lob, notes);

  Attributes::addDisplayName(speedMax, DisplayName::Vehicle::Rail::speedMax);
  Attributes::addEnabled(speedMax, editable);
  m_interfaceItems.insertBefore(speedMax, notes);

  Attributes::addDisplayName(weight, DisplayName::Vehicle::Rail::weight);
  Attributes::addEnabled(weight, editable);
  m_interfaceItems.insertBefore(weight, notes);

  Attributes::addObjectEditor(totalWeight, false);
  Attributes::addDisplayName(totalWeight, DisplayName::Vehicle::Rail::totalWeight);
  m_interfaceItems.insertBefore(totalWeight, notes);

  Attributes::addDisplayName(activeTrain, DisplayName::Vehicle::Rail::train); //TODO: "Active"
  Attributes::addEnabled(activeTrain, true);
  m_interfaceItems.insertBefore(activeTrain, notes);
}

void RailVehicle::addToWorld()
{
  Vehicle::addToWorld();
  m_world.railVehicles->addObject(shared_ptr<RailVehicle>());
}

void RailVehicle::destroying()
{
  if(decoder)
    decoder = nullptr;
  m_world.railVehicles->removeObject(shared_ptr<RailVehicle>());
  IdObject::destroying();
}

void RailVehicle::loaded()
{
  Vehicle::loaded();

  updateTotalWeight();
}

void RailVehicle::worldEvent(WorldState state, WorldEvent event)
{
  Vehicle::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(decoder, editable);
  Attributes::setEnabled(lob, editable);
  Attributes::setEnabled(speedMax, editable);
  Attributes::setEnabled(weight, editable);
}

double RailVehicle::calcTotalWeight(WeightUnit unit) const
{
  return weight.getValue(unit);
}

void RailVehicle::updateTotalWeight()
{
  totalWeight.setValueInternal(calcTotalWeight(totalWeight.unit()));
}
