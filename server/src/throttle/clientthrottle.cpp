/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2025 Reinder Feenstra
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

#include "clientthrottle.hpp"
#include "../core/attributes.hpp"
#include "../core/method.tpp"
#include "../core/objectproperty.tpp"
#include "../train/train.hpp"
#include "../world/world.hpp"

std::shared_ptr<ClientThrottle> ClientThrottle::create(World& world)
{
  auto obj = std::make_shared<ClientThrottle>(world, getUniqueLogId("client_throttle"));
  obj->addToList();
  return obj;
}

ClientThrottle::ClientThrottle(World& world, std::string_view objectId)
  : Throttle(world, objectId)
  , m_acquire{*this, "acquire", MethodFlags::NoScript,
      [this](const std::shared_ptr<Train>& acquireTrain, bool steal)
      {
        return acquire(acquireTrain, steal).value();
      }}
  , m_release{*this, "release", MethodFlags::NoScript, std::bind_front(&Throttle::release, this)}
  , m_emergencyStop{*this, "emergency_stop", MethodFlags::NoScript, std::bind_front(&Throttle::emergencyStop, this)}
  , m_setDirection{*this, "set_direction", MethodFlags::NoScript, std::bind_front(&Throttle::setDirection, this)}
  , m_setSpeed{*this, "set_speed", MethodFlags::NoScript,
      [this](double value, SpeedUnit unit, bool immediate)
      {
        return immediate ? setSpeed(value, unit) : setTargetSpeed(value, unit);
      }}
  , m_faster{*this, "faster", MethodFlags::NoScript, std::bind_front(&Throttle::faster, this)}
  , m_slower{*this, "slower", MethodFlags::NoScript, std::bind_front(&Throttle::slower, this)}
{
  Attributes::addObjectEditor(m_acquire, false);
  m_interfaceItems.add(m_acquire);

  Attributes::addObjectEditor(m_release, false);
  m_interfaceItems.add(m_release);

  Attributes::addObjectEditor(m_emergencyStop, false);
  m_interfaceItems.add(m_emergencyStop);

  Attributes::addObjectEditor(m_setDirection, false);
  m_interfaceItems.add(m_setDirection);

  Attributes::addObjectEditor(m_setSpeed, false);
  m_interfaceItems.add(m_setSpeed);

  Attributes::addObjectEditor(m_faster, false);
  m_interfaceItems.add(m_faster);

  Attributes::addObjectEditor(m_slower, false);
  m_interfaceItems.add(m_slower);
}
