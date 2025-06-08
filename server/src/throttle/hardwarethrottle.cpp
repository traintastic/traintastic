/**
 * server/src/throttle/hardwarethrottle.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2023,2025 Reinder Feenstra
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

#include "hardwarethrottle.hpp"
#include "../core/attributes.hpp"
#include "../core/errorcode.hpp"
#include "../core/objectproperty.tpp"
#include "../hardware/decoder/list/decoderlist.hpp"
#include "../train/train.hpp"
#include "../train/trainvehiclelist.hpp"
#include "../utils/displayname.hpp"
#include "../vehicle/rail/railvehicle.hpp"
#include "../world/world.hpp"

std::shared_ptr<HardwareThrottle> HardwareThrottle::create(std::shared_ptr<ThrottleController> controller, World& world)
{
  auto obj = std::make_shared<HardwareThrottle>(std::move(controller), world, getUniqueLogId());
  obj->addToList();
  return obj;
}

HardwareThrottle::HardwareThrottle(std::shared_ptr<ThrottleController> controller, World& world, std::string_view logId)
  : Throttle(world, logId)
  , interface{this, "interface", controller, PropertyFlags::ReadOnly | PropertyFlags::Store}
{
  Attributes::addDisplayName(interface, DisplayName::Hardware::interface);
  m_interfaceItems.add(interface);
}

std::error_code HardwareThrottle::acquire(DecoderProtocol protocol, uint16_t address, bool steal)
{
  assert(m_world.decoders.value());
  auto& decoderList = *m_world.decoders.value();

  auto decoder = decoderList.getDecoder(protocol, address);
  if(!decoder)
  {
    decoder = decoderList.getDecoder(address);
  }
  if(!decoder)
  {
    return make_error_code(ErrorCode::UnknownDecoderAddress);
  }
  if(!decoder->vehicle)
  {
    return make_error_code(ErrorCode::DecoderNotAssignedToAVehicle);
  }
  if(decoder->vehicle->activeTrain)
  {
    return Throttle::acquire(decoder->vehicle->activeTrain.value(), steal);
  }
  if(decoder->vehicle->trains.empty())
  {
    return make_error_code(ErrorCode::VehicleNotAssignedToATrain);
  }
  for(auto& vehicleTrain : decoder->vehicle->trains)
  {
    assert(!vehicleTrain->active);
    try
    {
      vehicleTrain->active = true; // try to activate train
      if(vehicleTrain->active)
      {
        return Throttle::acquire(vehicleTrain, steal);
      }
    }
    catch(...)
    {
    }
  }
  return make_error_code(ErrorCode::CanNotActivateTrain);
}

const std::shared_ptr<Decoder> HardwareThrottle::getDecoder(DecoderProtocol protocol, uint16_t address)
{
  static const std::shared_ptr<Decoder> noDecoder;

  if(!acquired())
  {
    return noDecoder;
  }

  // Find matching protocol and address:
  for(const auto& vehicle : *train->vehicles)
  {
    if(vehicle->decoder && vehicle->decoder->protocol == protocol && vehicle->decoder->address == address)
    {
      return vehicle->decoder.value();
    }
  }

  // Find matching addess, ignore protocol:
  for(const auto& vehicle : *train->vehicles)
  {
    if(vehicle->decoder && vehicle->decoder->address == address)
    {
      return vehicle->decoder.value();
    }
  }

  return noDecoder;
}

void HardwareThrottle::destroying()
{
  assert(interface);
  assert(m_world.hardwareThrottles > 0);
  if(interface->removeThrottle(*this))
    m_world.hardwareThrottles.setValueInternal(m_world.hardwareThrottles - 1);
  else
    assert(false);
  Throttle::destroying();
}

void HardwareThrottle::addToList()
{
  Throttle::addToList();
  assert(interface);
  if(interface->addThrottle(*this))
    m_world.hardwareThrottles.setValueInternal(m_world.hardwareThrottles + 1);
  else
    assert(false);
}
