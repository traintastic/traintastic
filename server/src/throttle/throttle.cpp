/**
 * server/src/throttle/throttle.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022,2025 Reinder Feenstra
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

#include "throttle.hpp"
#include "list/throttlelist.hpp"
#include "list/throttlelisttablemodel.hpp"
#include "../core/attributes.hpp"
#include "../core/method.tpp"
#include "../core/objectproperty.tpp"
#include "../core/objectvectorproperty.tpp"
#include "../hardware/decoder/decoder.hpp"
#include "../log/log.hpp"
#include "../train/train.hpp"
#include "../utils/displayname.hpp"
#include "../utils/valuestep.hpp"
#include "../world/world.hpp"

namespace {

constexpr double getValueStep(double value, SpeedUnit unit)
{
  switch(unit)
  {
    case SpeedUnit::KiloMeterPerHour:
      return (value >= 40.0) ? 5.0 : 2.0;

    case SpeedUnit::MilePerHour:
      return (value >= 30.0) ? 5.0 : 2.0;

    case SpeedUnit::MeterPerSecond:
      return (value >= 10.0) ? 1.0 : 0.5;
  }
  return 1;
}

}

std::unordered_set<std::string, StringHash, StringEqual> Throttle::s_logIds;

std::string_view Throttle::getUniqueLogId(std::string_view prefix)
{
  std::string uniqueLogId{prefix};
  uniqueLogId.append("_");
  uint32_t number = 0;
  do
  {
    uniqueLogId.resize(prefix.size() + 1);
    uniqueLogId.append(std::to_string(++number));
  }
  while(s_logIds.contains(uniqueLogId));
  return *s_logIds.emplace(std::move(uniqueLogId)).first;
}

Throttle::Throttle(World& world, std::string_view logId)
  : m_world{world}
  , m_logId{logId}
  , name{this, "name", std::string(logId), PropertyFlags::ReadWrite | PropertyFlags::NoStore | PropertyFlags::ScriptReadWrite}
  , train{this, "train", nullptr, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly}
  , onAcquire{*this, "on_acquire", EventFlags::Scriptable | EventFlags::Public}
  , onRelease{*this, "on_release", EventFlags::Scriptable | EventFlags::Public}
{
  Attributes::addDisplayName(name, DisplayName::Object::name);
  m_interfaceItems.add(name);

  Attributes::addObjectEditor(train, false);
  m_interfaceItems.add(train);
}

Throttle::~Throttle()
{
  assert(!acquired());
  if(auto it = s_logIds.find(m_logId); it != s_logIds.end()) [[likely]]
  {
    s_logIds.erase(it);
  }
}

bool Throttle::acquired() const
{
  return train.operator bool();
}

std::error_code Throttle::acquire(const std::shared_ptr<Train>& acquireTrain, bool steal)
{
  assert(acquireTrain);
  const auto stole = steal && acquireTrain->hasThrottle();
  const std::string stoleFrom = stole ? acquireTrain->throttleName() : std::string{};
  const auto ec = acquireTrain->acquire(*this, steal);
  if(ec)
  {
    Log::log(m_logId, LogMessage::D3001_ACQUIRING_TRAIN_X_FAILED_X, acquireTrain->name.value(), ec.message());
    return ec;
  }

  if(acquired())
  {
    release();
  }

  assert(!train);
  train.setValueInternal(acquireTrain);

  if(stole)
  {
    Log::log(m_logId, LogMessage::N3005_THROTTLE_X_STOLE_TRAIN_X_FROM_THROTTLE_X, name.value(), train->name.value(), stoleFrom);
  }
  else
  {
    Log::log(m_logId, LogMessage::I3001_THROTTLE_X_ACQUIRED_TRAIN_X, name.value(), train->name.value());
  }

  fireEvent(onAcquire, shared_ptr<Throttle>(), acquireTrain);

  return {};
}

void Throttle::release(bool stopIt)
{
  if(!acquired())
    return;

  if(stopIt)
  {
    emergencyStop();
  }

  const auto logMessage = !stopIt && !train->isStopped.value() ? LogMessage::N3006_THROTTLE_X_RELEASED_TRAIN_X_WITHOUT_STOPPING_IT : LogMessage::I3002_THROTTLE_X_RELEASED_TRAIN_X;
  Log::log(m_logId, logMessage, name.value(), train->name.value());
  train->release(*this);
  train.setValueInternal(nullptr);

  fireEvent(onRelease, shared_ptr<Throttle>());
}

bool Throttle::emergencyStop()
{
  if(acquired())
  {
    train->emergencyStop = true;
    return true;
  }
  return false;
}

bool Throttle::setDirection(Direction value)
{
  if(acquired() && (value == Direction::Forward || value == Direction::Reverse))
  {
    return !train->setDirection(*this, value);
  }
  return false;
}

bool Throttle::setSpeed(double value, SpeedUnit unit)
{
  if(acquired())
  {
    train->emergencyStop = false;
    return !train->setSpeed(*this, convertUnit(value, unit, train->speed.unit()));
  }
  return false;
}

bool Throttle::setTargetSpeed(double value, SpeedUnit unit)
{
  if(acquired())
  {
    train->emergencyStop = false;
    return !train->setTargetSpeed(*this, convertUnit(value, unit, train->throttleSpeed.unit()));
  }
  return false;
}

bool Throttle::slower(bool immediate)
{
  if(acquired())
  {
    train->emergencyStop = false;
    if(immediate)
    {
      const double value = train->speed.value();
      const double step = getValueStep(value, train->speed.unit());
      return !train->setSpeed(*this, valueStepDown(value, step));
    }
    const double value = train->throttleSpeed.value();
    const double step = getValueStep(value, train->throttleSpeed.unit());
    return !train->setTargetSpeed(*this, valueStepDown(value, step));
  }
  return false;
}

bool Throttle::faster(bool immediate)
{
  if(acquired())
  {
    train->emergencyStop = false;
    if(immediate)
    {
      const double value = train->speed.value();
      const double step = getValueStep(value, train->speed.unit());
      return !train->setSpeed(*this, valueStepUp(value, step));
    }
    const double value = train->throttleSpeed.value();
    const double step = getValueStep(value, train->throttleSpeed.unit());
    return !train->setTargetSpeed(*this, valueStepUp(value, step));
  }
  return false;
}

void Throttle::destroying()
{
  m_world.throttles->removeObject(shared_ptr<Throttle>());
  release();
  NonPersistentObject::destroying();
}

void Throttle::addToList()
{
  m_world.throttles->addObject(shared_ptr<Throttle>());
}
