/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#include "autothrottle.hpp"
#include "../core/attributes.hpp"
#include "../core/method.tpp"
#include "../core/objectproperty.tpp"
#include "../train/train.hpp"
#include "../world/world.hpp"

std::shared_ptr<AutoThrottle> AutoThrottle::assign(const std::shared_ptr<Train>& train, bool steal, std::error_code& ec)
{
  auto& world = train->world();
  std::shared_ptr<AutoThrottle> throttle(new AutoThrottle(world, getUniqueLogId("auto_throttle")));
  ec = throttle->acquire(train, steal);
  if(ec)
  {
    return {};
  }
  return throttle;
}

AutoThrottle::AutoThrottle(World& world, std::string_view objectId)
  : Throttle(world, objectId)
  , state{this, "state", AutoThrottleState::Idle, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly}
  , reason{this, "reason", AutoThrottleReason::None, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly}
{
  Attributes::addValues(state, autoThrottleStateValues);
  m_interfaceItems.add(state);

  Attributes::addValues(reason, autoThrottleReasonValues);
  m_interfaceItems.add(reason);
}

void AutoThrottle::worldEvent(WorldState worldState, WorldEvent worldEvent)
{
  Throttle::worldEvent(worldState, worldEvent);

  switch(worldEvent)
  {
    case WorldEvent::PowerOff:
    case WorldEvent::PowerOn:
    case WorldEvent::Stop:
    case WorldEvent::Run:
    case WorldEvent::AutomaticDisabled:
    case WorldEvent::AutomaticEnabled:
      evaluate();
      break;

    default:
      break;
  }
}

void AutoThrottle::trainChanged()
{
  if(train) // train acquired
  {
    auto onRouteEvent =
      [this](const std::shared_ptr<Train>&, const std::shared_ptr<TrainRoute>&)
      {
        evaluate();
      };
    m_trainEvents.emplace_back(train->onRouteAssigned.connect(onRouteEvent));
    m_trainEvents.emplace_back(train->onRouteCanceled.connect(onRouteEvent));
    m_trainEvents.emplace_back(train->onRouteCompleted.connect(onRouteEvent));
    evaluate();
  }
  else
  {
    m_trainEvents.clear();
  }
}

void AutoThrottle::evaluate()
{
  assert(train);

  switch(state.value())
  {
    case AutoThrottleState::Idle:
      evaluateIdle();
      break;
  }
}

void AutoThrottle::evaluateIdle()
{
  assert(state.value() == AutoThrottleState::Idle);

  if(!train->isStopped || train->emergencyStop)
  {
    train->emergencyStop = false;
    train->stop();
  }

  if(!contains(m_world.state.value(), WorldState::PowerOn))
  {
    reason.setValueInternal(AutoThrottleReason::PowerOff);
    return;
  }
  if(!contains(m_world.state.value(), WorldState::Run))
  {
    reason.setValueInternal(AutoThrottleReason::WorldStop);
    return;
  }
  if(!m_world.automatic)
  {
    reason.setValueInternal(AutoThrottleReason::AutomaticDisabled);
    return;
  }
  if(!train->route)
  {
    reason.setValueInternal(AutoThrottleReason::NoRoute);
    return;
  }
  //if(...)
  {
    reason.setValueInternal(AutoThrottleReason::WaitingForPath);
    return;
  }
}
