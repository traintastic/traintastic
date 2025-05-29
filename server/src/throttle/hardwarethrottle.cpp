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
#include "../core/objectproperty.tpp"
#include "../hardware/decoder/list/decoderlist.hpp"
#include "../train/train.hpp"
#include "../utils/displayname.hpp"
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

Throttle::AcquireResult HardwareThrottle::acquire(DecoderProtocol protocol, uint16_t address, bool steal)
{
  assert(m_world.decoders.value());
  auto& decoderList = *m_world.decoders.value();

  auto decoder = decoderList.getDecoder(protocol, address);
  if(!decoder)
    decoder = decoderList.getDecoder(address);
  if(!decoder)
    return AcquireResult::FailedNonExisting;

  return Throttle::acquire(std::move(decoder), steal);
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
