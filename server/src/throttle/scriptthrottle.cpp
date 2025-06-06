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

#include "scriptthrottle.hpp"
#include "../core/attributes.hpp"
#include "../core/method.tpp"
#include "../core/objectproperty.tpp"
#include "../train/train.hpp"
#include "../world/world.hpp"

std::shared_ptr<ScriptThrottle> ScriptThrottle::create(World& world)
{
  auto obj = std::make_shared<ScriptThrottle>(world, getUniqueLogId("scriptthrottle"));
  obj->addToList();
  return obj;
}

ScriptThrottle::ScriptThrottle(World& world, std::string_view objectId)
  : Throttle(world, objectId)
  , m_emergencyStop{*this, "emergency_stop", MethodFlags::ScriptCallable, std::bind_front(&Throttle::emergencyStop, this)}
  , m_setDirection{*this, "set_direction", MethodFlags::ScriptCallable, std::bind_front(&Throttle::setDirection, this)}
  , m_changeDirection{*this, "change_direction", MethodFlags::ScriptCallable,
      [this]()
      {
        return acquired() && setDirection(~train->direction.value());
      }}
  , m_setSpeed{*this, "set_speed", MethodFlags::ScriptCallable, std::bind_front(&Throttle::setSpeed, this)}
  , m_setTargetSpeed{*this, "set_target_speed", MethodFlags::ScriptCallable, std::bind_front(&Throttle::setTargetSpeed, this)}
{
  Attributes::addObjectEditor(m_emergencyStop, false);
  m_interfaceItems.add(m_emergencyStop);

  Attributes::addObjectEditor(m_setDirection, false);
  m_interfaceItems.add(m_setDirection);

  Attributes::addObjectEditor(m_changeDirection, false);
  m_interfaceItems.add(m_changeDirection);

  Attributes::addObjectEditor(m_setSpeed, false);
  m_interfaceItems.add(m_setSpeed);

  Attributes::addObjectEditor(m_setTargetSpeed, false);
  m_interfaceItems.add(m_setTargetSpeed);
}
