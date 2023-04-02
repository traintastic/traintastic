/**
 * server/src/vehicle/rail/poweredrailvehicle.cpp
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

#include "poweredrailvehicle.hpp"
#include "../../core/attributes.hpp"
#include "../../core/objectproperty.tpp"
#include "../../utils/almostzero.hpp"
#include "../../utils/displayname.hpp"
#include "../../world/world.hpp"
#include "../../hardware/decoder/decoder.hpp"
#include "../../hardware/decoder/decoderchangeflags.hpp"
#include "../../train/train.hpp"

PoweredRailVehicle::PoweredRailVehicle(World& world, std::string_view id_)
  : RailVehicle(world, id_)
  , power{*this, "power", 0, PowerUnit::KiloWatt, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  const bool editable = contains(m_world.state.value(), WorldState::Edit);

  Attributes::addDisplayName(power, DisplayName::Vehicle::Rail::power);
  Attributes::addEnabled(power, editable);
  m_interfaceItems.add(power);

  propertyChanged.connect(
    [this](BaseProperty &prop)
    {
      if(prop.name() == "decoder")
        registerDecoder();
    });
}

PoweredRailVehicle::~PoweredRailVehicle()
{
  assert(!decoder);
  assert(!decoderConnection.connected());
}

void PoweredRailVehicle::destroying()
{
  decoder = nullptr;
  RailVehicle::destroying();
}

void PoweredRailVehicle::loaded()
{
  RailVehicle::loaded();
  registerDecoder();
}

void PoweredRailVehicle::setDirection(Direction value)
{
  if(decoder)
    decoder->direction = value;
}

void PoweredRailVehicle::setEmergencyStop(bool value)
{
  if(decoder)
    decoder->emergencyStop = value;
}

void PoweredRailVehicle::setSpeed(double kmph)
{
  if(!decoder)
    return;

  if(almostZero(kmph))
  {
    decoder->throttle.setValue(0);
    return;
  }

  //! \todo Implement speed profile

  // No speed profile -> linear
  {
    const double max = speedMax.getValue(SpeedUnit::KiloMeterPerHour);
    float val = 0;
    if(max > 0)
    {
      const uint8_t steps = decoder->speedSteps;
      if(steps == Decoder::speedStepsAuto)
        val = kmph / max;
      else
        val = std::round(kmph / max * steps) / steps;
    }

    //Remember last speed set by train, see lambda in 'PoweredRailVehicle::registerDecoder()'
    lastTrainSpeedStep = val;
    decoder->throttle.setValue(val);
  }
}

void PoweredRailVehicle::worldEvent(WorldState state, WorldEvent event)
{
  RailVehicle::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(power, editable);
}

void PoweredRailVehicle::registerDecoder()
{
  //Disconnect from previous decoder
  decoderConnection.disconnect();

  auto decoderVal = decoder.value();
  if(!decoderVal)
    return;

  //Connect to new decoder
  decoderConnection = decoderVal->decoderChanged.connect(
    [this](Decoder& self, DecoderChangeFlags flags, uint32_t /*functionNumber*/)
    {
      if(!activeTrain)
        return;

      if(flags == DecoderChangeFlags::EmergencyStop)
      {
        activeTrain.value()->emergencyStop.setValue(self.emergencyStop);
      }
      else if(flags == DecoderChangeFlags::Throttle)
      {
        if(almostZero(lastTrainSpeedStep - self.throttle))
        {
          //When train speed changes, decoder throttle is updated
          //Do not update train speed back when this happens
          //Otherwise an infinite recursion would be triggered
          return;
        }

        //! \todo Implement speed profile

        // No speed profile -> linear
        const double kmph = self.throttle * speedMax.getValue(SpeedUnit::KiloMeterPerHour);
        activeTrain.value()->throttleSpeed.setValue(kmph);
      }
    });
}
