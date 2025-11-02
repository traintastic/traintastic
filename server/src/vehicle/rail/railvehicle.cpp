/**
 * server/src/vehicle/rail/railvehicle.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021,2023-2025 Reinder Feenstra
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
#include "../../hardware/decoder/list/decoderlist.hpp"
#include "../../world/world.hpp"
#include "../../core/attributes.hpp"
#include "../../core/objectproperty.tpp"
#include "../../core/objectvectorproperty.tpp"
#include "../../core/method.tpp"
#include "../../core/controllerlist.hpp"
#include "../../utils/displayname.hpp"
#include "../../train/train.hpp"
#include "../../train/trainvehiclelist.hpp"
#include "../../hardware/decoder/decodercontroller.hpp"

RailVehicle::RailVehicle(World& world, std::string_view _id) :
  Vehicle(world, _id),
  decoder{this, "decoder", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store},
  length{*this, "length", 0, LengthUnit::MilliMeter, PropertyFlags::ReadWrite | PropertyFlags::Store},
  speedMax{*this, "speed_max", 0, SpeedUnit::KiloMeterPerHour, PropertyFlags::ReadWrite | PropertyFlags::Store},
  weight{*this, "weight", 0, WeightUnit::Ton, PropertyFlags::ReadWrite | PropertyFlags::Store, [this](double /*value*/, WeightUnit /*unit*/){ updateTotalWeight(); }},
  totalWeight{*this, "total_weight", 0, WeightUnit::Ton, PropertyFlags::ReadOnly | PropertyFlags::NoStore}
  , mute{this, "mute", false, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly}
  , noSmoke{this, "no_smoke", false, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly}
  , activeTrain{this, "active_train", nullptr, PropertyFlags::ReadOnly | PropertyFlags::ScriptReadOnly | PropertyFlags::StoreState}
  , trains{*this, "trains", {}, PropertyFlags::ReadOnly | PropertyFlags::ScriptReadOnly | PropertyFlags::NoStore}
  , createDecoder{*this, "create_decoder", MethodFlags::NoScript,
      [this]()
      {
        if(!decoder)
        {
          decoder.setValueInternal(Decoder::create(m_world));
          decoder->vehicle.setValueInternal(shared_ptr<RailVehicle>());
          if(m_world.decoderControllers->length == 1)
          {
            decoder->interface = std::dynamic_pointer_cast<DecoderController>(m_world.decoderControllers->getObject(0));
          }
          decoder->functions->create(); // add FL/F0 light function
        }
      }}
  , deleteDecoder{*this, "delete_decoder", MethodFlags::NoScript,
      [this]()
      {
        if(decoder)
        {
          decoder->destroy();
          assert(!decoder);
        }
      }}
{
  const bool editable = contains(m_world.state.value(), WorldState::Edit);

  Attributes::addDisplayName(decoder, DisplayName::Vehicle::Rail::decoder);
  Attributes::addEnabled(decoder, editable);
  Attributes::addObjectList(decoder, m_world.decoders);
  m_interfaceItems.insertBefore(decoder, notes);

  Attributes::addDisplayName(length, DisplayName::Vehicle::Rail::length);
  Attributes::addEnabled(length, editable);
  m_interfaceItems.insertBefore(length, notes);

  Attributes::addDisplayName(speedMax, DisplayName::Vehicle::Rail::speedMax);
  Attributes::addEnabled(speedMax, editable);
  m_interfaceItems.insertBefore(speedMax, notes);

  Attributes::addDisplayName(weight, DisplayName::Vehicle::Rail::weight);
  Attributes::addEnabled(weight, editable);
  m_interfaceItems.insertBefore(weight, notes);

  Attributes::addObjectEditor(totalWeight, false);
  Attributes::addDisplayName(totalWeight, DisplayName::Vehicle::Rail::totalWeight);
  m_interfaceItems.insertBefore(totalWeight, notes);

  Attributes::addObjectEditor(mute, false);
  m_interfaceItems.add(mute);

  Attributes::addObjectEditor(noSmoke, false);
  m_interfaceItems.add(noSmoke);

  Attributes::addDisplayName(activeTrain, DisplayName::Vehicle::Rail::train); //TODO: "Active"
  Attributes::addEnabled(activeTrain, true);
  Attributes::addObjectEditor(activeTrain, false);
  m_interfaceItems.insertBefore(activeTrain, notes);

  Attributes::addObjectEditor(trains, false);
  m_interfaceItems.insertBefore(trains, notes);

  Attributes::addObjectEditor(createDecoder, false);
  m_interfaceItems.add(createDecoder);

  Attributes::addObjectEditor(deleteDecoder, false);
  m_interfaceItems.add(deleteDecoder);
}

void RailVehicle::setActiveTrain(const std::shared_ptr<Train>& train)
{
  activeTrain.setValueInternal(train);
  updateMute();
  updateNoSmoke();
}

void RailVehicle::updateMute()
{
  bool value = contains(m_world.state, WorldState::NoSmoke);
  if(!value && activeTrain)
  {
    value |= activeTrain->mute;
  }
  if(value != mute)
  {
    mute.setValueInternal(value);
    if(decoder)
    {
      decoder->updateMute();
    }
  }
}

void RailVehicle::updateNoSmoke()
{
  bool value = contains(m_world.state, WorldState::NoSmoke);
  if(!value && activeTrain)
  {
    value |= activeTrain->noSmoke;
  }
  if(value != noSmoke)
  {
    noSmoke.setValueInternal(value);
    if(decoder)
    {
      decoder->updateNoSmoke();
    }
  }
}

void RailVehicle::addToWorld()
{
  Vehicle::addToWorld();
  m_world.railVehicles->addObject(shared_ptr<RailVehicle>());
}

void RailVehicle::destroying()
{
  auto self = shared_ptr<RailVehicle>();
  if(decoder)
  {
    decoder->destroy();
    assert(!decoder);
  }
  for(const auto& train : trains)
  {
    train->vehicles->remove(self);
  }
  m_world.railVehicles->removeObject(self);
  IdObject::destroying();
}

void RailVehicle::loaded()
{
  Vehicle::loaded();

  if(decoder)
  {
    decoder->vehicle.setValueInternal(shared_ptr<RailVehicle>());
  }

  updateTotalWeight();
}

void RailVehicle::worldEvent(WorldState state, WorldEvent event)
{
  Vehicle::worldEvent(state, event);

  switch(event)
  {
    case WorldEvent::EditEnabled:
    case WorldEvent::EditDisabled:
    {
      const bool editable = contains(state, WorldState::Edit);
      Attributes::setEnabled(decoder, editable);
      Attributes::setEnabled(length, editable);
      Attributes::setEnabled(speedMax, editable);
      Attributes::setEnabled(weight, editable);
      break;
    }
    case WorldEvent::Mute:
    case WorldEvent::Unmute:
      updateMute();
      break;

    case WorldEvent::NoSmoke:
    case WorldEvent::Smoke:
      updateNoSmoke();
      break;

    default:
      break;
  }
}

double RailVehicle::calcTotalWeight(WeightUnit unit) const
{
  return weight.getValue(unit);
}

void RailVehicle::updateTotalWeight()
{
  totalWeight.setValueInternal(calcTotalWeight(totalWeight.unit()));
}
