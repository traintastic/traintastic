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
#include "vehiclespeedcurve.hpp"
#include "../../core/attributes.hpp"
#include "../../core/objectproperty.tpp"
#include "../../core/method.tpp"
#include "../../utils/almostzero.hpp"
#include "../../utils/displayname.hpp"
#include "../../world/world.hpp"
#include "../../hardware/decoder/decoder.hpp"
#include "../../hardware/decoder/decoderchangeflags.hpp"
#include "../../train/train.hpp"

PoweredRailVehicle::PoweredRailVehicle(World& world, std::string_view id_)
  : RailVehicle(world, id_)
  , power{*this, "power", 0, PowerUnit::KiloWatt, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , speedCurve{this, "speed_curve", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::Store | PropertyFlags::ScriptReadOnly}
  , maxAccelerationRate{this, "max_acceleration_rate", 3.0, PropertyFlags::ReadWrite | PropertyFlags::ScriptReadOnly | PropertyFlags::Store}
  , maxBrakingRate{this, "max_braking_rate", 2.0, PropertyFlags::ReadWrite | PropertyFlags::ScriptReadOnly | PropertyFlags::Store}
{
  const bool editable = contains(m_world.state.value(), WorldState::Edit);

  Attributes::addDisplayName(power, DisplayName::Vehicle::Rail::power);
  Attributes::addEnabled(power, editable);
  m_interfaceItems.add(power);

  speedCurve.setValueInternal(std::make_shared<VehicleSpeedCurve>(*this, speedCurve.name()));
  Attributes::addEnabled(speedCurve, editable);
  m_interfaceItems.add(speedCurve);

  Attributes::addDisplayName(maxAccelerationRate, DisplayName::Vehicle::Rail::maxAccelerationRate);
  Attributes::addEnabled(maxAccelerationRate, editable);
  Attributes::addMinMax(maxAccelerationRate, 0.01, 10.0); // m/s^2
  m_interfaceItems.add(maxAccelerationRate);

  Attributes::addDisplayName(maxBrakingRate, DisplayName::Vehicle::Rail::maxBrakingRate);
  Attributes::addEnabled(maxBrakingRate, editable);
  Attributes::addMinMax(maxBrakingRate, -10.0, 0.01); // m/s^2
  m_interfaceItems.add(maxBrakingRate);

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

  // decoder propertyChanged is not emitted when object is dying
  // So disconnect manually
  decoderConnection.disconnect();
  RailVehicle::destroying();
}

void PoweredRailVehicle::loaded()
{
  RailVehicle::loaded();
  updateMaxSpeed();
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

void PoweredRailVehicle::worldEvent(WorldState state, WorldEvent event)
{
  RailVehicle::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(power, editable);
  Attributes::setEnabled(speedCurve, editable);
  Attributes::setEnabled(maxAccelerationRate, editable);
  Attributes::setEnabled(maxBrakingRate, editable);
}

void PoweredRailVehicle::updateMaxSpeed()
{
  propertyChanged(speedCurve); //TODO: find better way

  if(!speedCurve->isValid())
    return;

  double speedMS = speedCurve->getSpeedForStep(126);
  speedMS *= m_world.scaleRatio;

  speedMax.setValueInternal(convertUnit(speedMS,
                                        SpeedUnit::MeterPerSecond,
                                        speedMax.unit()));
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

      if(has(flags, DecoderChangeFlags::Direction))
      {
        if(self.direction == lastTrainSetDirection)
          return; //Direction change was caused by Train itself, no need propagate back
        activeTrain->handleDecoderDirection(this->shared_ptr<PoweredRailVehicle>(), self.direction);
      }

      if(has(flags, DecoderChangeFlags::EmergencyStop))
      {
        activeTrain->emergencyStop.setValue(self.emergencyStop);
      }
      else if(has(flags, DecoderChangeFlags::Throttle))
      {
        if(almostZero(lastTrainSpeedStep - self.throttle))
        {
          //When train speed changes, decoder throttle is updated
          //Do not update train speed back when this happens
          //Otherwise an infinite recursion would be triggered
          return;
        }

        activeTrain->handleDecoderThrottle(shared_ptr<PoweredRailVehicle>(),
                                           self.throttle);
      }
    });
}
