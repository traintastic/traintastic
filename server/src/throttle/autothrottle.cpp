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
#include "../board/map/blockpath.hpp"
#include "../board/tile/rail/blockrailtile.hpp"
#include "../core/attributes.hpp"
#include "../core/method.tpp"
#include "../core/objectproperty.tpp"
#include "../log/log.hpp"
#include "../route/trainroute.hpp"
#include "../route/trainrouteentry.hpp"
#include "../train/train.hpp"
#include "../train/trainblockstatus.hpp"
#include "../world/world.hpp"

namespace {

const std::shared_ptr<TrainRouteEntry> noRouteEntry;

constexpr BlockSide entrySide(BlockTrainDirection direction)
{
  return (direction == BlockTrainDirection::TowardsA) ? BlockSide::B : BlockSide::A;
}

constexpr BlockSide exitSide(BlockTrainDirection direction)
{
  return ~entrySide(direction);
}

const std::shared_ptr<TrainRouteEntry>& routeEntry(const Train& train, uint32_t position)
{
  if(train.route)
  {
    const auto& entriesResolved = train.route->entriesResolved;
    if(position < entriesResolved.size())
    {
      return entriesResolved[position];
    }
  }
  return noRouteEntry;
}

const std::shared_ptr<TrainRouteEntry>& currentRouteEntry(const Train& train)
{
  return routeEntry(train, train.routePosition);
}

const std::shared_ptr<TrainRouteEntry>& nextRouteEntry(const Train& train)
{
  return routeEntry(train, train.routePosition + 1);
}

bool hasNextRouteEntry(Train& train)
{
  return train.route && (train.routePosition < train.route->entriesResolved.size() - 1);
}

std::shared_ptr<BlockPath> getNextPath(Train& train)
{
  if(!train.route || train.blocks.empty())
  {
    return {};
  }

  const auto& nextEntry = nextRouteEntry(train);
  if(!nextEntry)
  {
    return {};
  }

  const auto toSide = entrySide(nextEntry->blockTrainDirection());

  for(auto side : {exitSide(train.blocks[0]->direction), entrySide(train.blocks[0]->direction)})
  {
    auto path = train.blocks[0]->block->getReservedPath(side);
    if(path && path->toBlock() == nextEntry->block.value() && path->toSide() == toSide)
    {
      return path;
    }
  }

  return {};
}

}

std::shared_ptr<AutoThrottle> AutoThrottle::assign(const std::shared_ptr<Train>& train, bool steal, std::error_code& ec)
{
  auto& world = train->world();
  std::shared_ptr<AutoThrottle> throttle(new AutoThrottle(world, getUniqueLogId("auto_throttle")));
  ec = throttle->acquire(train, steal);
  if(ec)
  {
    return {};
  }
  throttle->addToList();
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

void AutoThrottle::trainChanged()
{
  if(train) // train acquired
  {
    m_events.emplace_back(m_world.onEvent.connect(
      [this](WorldState /*worldState*/, WorldEvent worldEvent)
      {
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
      }));

    m_events.emplace_back(train->onBlockEntered.connect(
      [this](const std::shared_ptr<Train>&, const std::shared_ptr<BlockRailTile>&, BlockTrainDirection)
      {
        evaluate();
      }));

    auto onRouteEvent =
      [this](const std::shared_ptr<Train>&, const std::shared_ptr<TrainRoute>&)
      {
        evaluate();
      };
    m_events.emplace_back(train->onRouteAssigned.connect(onRouteEvent));
    m_events.emplace_back(train->onRouteCanceled.connect(onRouteEvent));
    m_events.emplace_back(train->onRouteCompleted.connect(onRouteEvent));

    evaluate();
  }
  else
  {
    m_events.clear();
    destroy();
  }
}

void AutoThrottle::changeState(AutoThrottleState newState, AutoThrottleReason newReason)
{
  assert(state != newState);
  state.setValueInternal(newState);
  reason.setValueInternal(newReason);
  evaluate();
}

void AutoThrottle::evaluate()
{
  assert(train);

  switch(state.value())
  {
    case AutoThrottleState::Idle:
      LOG_DEBUG(name.value(), "state = Idle");
      evaluateIdle();
      break;

    case AutoThrottleState::Depart:
      LOG_DEBUG(name.value(), "state = Depart");
      evaluateDepart();
      break;

    case AutoThrottleState::Drive:
      LOG_DEBUG(name.value(), "state = Drive");
      evaluateDrive();
      break;

    case AutoThrottleState::Arrive:
      LOG_DEBUG(name.value(), "state = Arrive");
      evaluateArrive();
      break;
  }
}

void AutoThrottle::evaluateIdle()
{
  assert(state.value() == AutoThrottleState::Idle);

  if(!contains(m_world.state.value(), WorldState::PowerOn))
  {
    LOG_DEBUG(name.value(), "reason = PowerOff");
    stop();
    reason.setValueInternal(AutoThrottleReason::PowerOff);
    return;
  }
  if(!contains(m_world.state.value(), WorldState::Run))
  {
    LOG_DEBUG(name.value(), "reason = WorldStop");
    stop();
    reason.setValueInternal(AutoThrottleReason::WorldStop);
    return;
  }
  if(!m_world.automatic)
  {
    LOG_DEBUG(name.value(), "reason = AutomaticDisabled");
    stop();
    reason.setValueInternal(AutoThrottleReason::AutomaticDisabled);
    return;
  }
  if(!train->route)
  {
    LOG_DEBUG(name.value(), "reason = NoRoute");
    stop();
    reason.setValueInternal(AutoThrottleReason::NoRoute);
    return;
  }

  const auto& entry = currentRouteEntry(*train);
  if(!train->route->valid || !entry)
  {
    LOG_DEBUG(name.value(), "reason = InvalidRoute");
    stop();
    reason.setValueInternal(AutoThrottleReason::InvalidRoute);
    return;
  }
  if(train->blocks[0]->direction != entry->blockTrainDirection())
  {
    LOG_DEBUG(name.value(), "reason = WrongDirection");
    setDirection(~train->direction.value());
    reason.setValueInternal(AutoThrottleReason::WrongDirection);
    return;
  }

  auto path = getNextPath(*train);
  if(!path || !path->isReady())
  {
    LOG_DEBUG(name.value(), "reason = WaitingForPath");
    stop();
    reason.setValueInternal(AutoThrottleReason::WaitingForPath);
    return;
  }

  changeState(train->isStopped ? AutoThrottleState::Depart : AutoThrottleState::Drive);
}

void AutoThrottle::evaluateDepart()
{
  assert(state.value() == AutoThrottleState::Depart);

  if(!contains(m_world.state.value(), WorldState::Run))
  {
    stop();
    changeState(AutoThrottleState::Idle);
    return;
  }

  setTargetSpeed(train->speedMax.value() * 0.5, train->speedMax.unit()); // FIXME: hardcoded for initial testing
  changeState(AutoThrottleState::Drive);
}

void AutoThrottle::evaluateDrive()
{
  assert(state.value() == AutoThrottleState::Drive);

  if(!contains(m_world.state.value(), WorldState::Run))
  {
    stop();
    changeState(AutoThrottleState::Idle);
    return;
  }

  // FIXME: handle route entry wait

  if(!hasNextRouteEntry(*train)) // end of route
  {
    changeState(AutoThrottleState::Arrive);
    return;
  }

  auto path = getNextPath(*train);
  if(!path || !path->isReady()) // no path to continue
  {
    changeState(AutoThrottleState::Arrive);
    return;
  }
}

void AutoThrottle::evaluateArrive()
{
  assert(state.value() == AutoThrottleState::Arrive);

  if(!contains(m_world.state.value(), WorldState::Run))
  {
    stop();
    changeState(AutoThrottleState::Idle);
    return;
  }

  if(train->isStopped)
  {
    changeState(AutoThrottleState::Idle);
    return;
  }

  train->stop(); // FIXME: delay stop until train is in block (if that fits)
}

void AutoThrottle::stop()
{
  if(!train->isStopped || train->emergencyStop)
  {
    train->emergencyStop = false;
    setSpeed(0.0, train->speedMax.unit());
  }
}
