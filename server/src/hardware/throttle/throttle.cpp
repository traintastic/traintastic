/**
 * server/src/hardware/throttle/throttle.cpp
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
#include "../../core/attributes.hpp"
#include "../../core/method.tpp"
#include "../../core/objectproperty.tpp"
#include "../../core/objectvectorproperty.tpp"
#include "../../hardware/decoder/decoder.hpp"
#include "../../log/log.hpp"
#include "../../train/train.hpp"
#include "../../utils/displayname.hpp"
#include "../../utils/valuestep.hpp"
#include "../../world/world.hpp"

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

Throttle::Throttle(World& world, std::string_view _id)
  : IdObject(world, _id)
  , name{this, "name", id, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , direction{this, "direction", Direction::Forward, PropertyFlags::ReadOnly}
  , throttle{this, "throttle", throttleMin, PropertyFlags::ReadWrite,
      [this](const float& value)
      {
        if(m_decoder)
          m_decoder->throttle = value;
      },
      [this](float& /*value*/) -> bool
      {
        return acquired();
      }}
  , train{this, "train", nullptr, PropertyFlags::ReadOnly}
  , emergencyStop{*this, "emergency_stop",
      [this]()
      {
        if(acquired())
        {
          if(train)
          {
            train->emergencyStop = true;
          }
          if(m_decoder)
          {
            m_decoder->emergencyStop = true;
          }
          return true;
        }
        return false;
      }}
  , stop{*this, "stop",
      [this]()
      {
        if(acquired())
        {
          if(train)
          {
            train->emergencyStop = false;
            train->stop();
          }
          if(m_decoder)
          {
            m_decoder->emergencyStop = false;
            m_decoder->throttle = throttleStop;
          }
          return true;
        }
        return false;
      }}
  , faster{*this, "faster",
      [this]()
      {
        if(acquired())
        {
          if(train)
          {
            const double value = train->throttleSpeed.value();
            const double step = getValueStep(value, train->throttleSpeed.unit());
            train->emergencyStop = false;
            train->setTargetSpeed(*this, valueStepUp(value, step));
          }
          return true;
        }
        return false;
      }}
  , slower{*this, "slower",
      [this]()
      {
        if(acquired())
        {
          if(train)
          {
            const double value = train->throttleSpeed.value();
            const double step = getValueStep(value, train->throttleSpeed.unit());
            train->emergencyStop = false;
            train->setTargetSpeed(*this, valueStepDown(value, step));
          }
          return true;
        }
        return false;
      }}
  , setDirection{*this, "set_direction",
      [this](Direction value)
      {
        if(acquired() && (value == Direction::Forward || value == Direction::Reverse))
        {
          if(train)
          {
            return !train->setDirection(*this, value);
          }
          if(m_decoder)
          {
            m_decoder->direction = value;
          }
          return true;
        }
        return false;
      }}
{
  const bool editable = contains(m_world.state.value(), WorldState::Edit);

  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);

  Attributes::addEnabled(direction, false);
  Attributes::addValues(direction, DirectionValues);
  Attributes::addObjectEditor(direction, false);
  m_interfaceItems.add(direction);

  Attributes::addEnabled(throttle, false);
  Attributes::addMinMax(throttle, throttleMin, throttleMax);
  Attributes::addObjectEditor(throttle, false);
  m_interfaceItems.add(throttle);

  Attributes::addObjectEditor(train, false);
  m_interfaceItems.add(train);

  Attributes::addEnabled(emergencyStop, false);
  Attributes::addObjectEditor(emergencyStop, false);
  m_interfaceItems.add(emergencyStop);

  Attributes::addEnabled(stop, false);
  Attributes::addObjectEditor(stop, false);
  m_interfaceItems.add(stop);

  Attributes::addEnabled(setDirection, false);
  Attributes::addObjectEditor(setDirection, false);
  m_interfaceItems.add(setDirection);
}

#ifndef NDEBUG
Throttle::~Throttle()
{
  assert(!acquired());
}
#endif

bool Throttle::acquired() const
{
  return m_decoder.operator bool() || train.operator bool();
}

std::error_code Throttle::acquire(const std::shared_ptr<Train>& acquireTrain, bool steal)
{
  assert(acquireTrain);
  const auto stole = steal && acquireTrain->hasThrottle();
  const std::string stoleFrom = stole ? acquireTrain->throttleName() : std::string{};
  const auto ec = acquireTrain->acquire(*this, steal);
  if(ec)
  {
    Log::log(*this, LogMessage::D3001_ACQUIRING_TRAIN_X_FAILED_X, acquireTrain->name.value(), ec.message());
    return ec;
  }

  train.setValueInternal(acquireTrain);

  if(stole)
  {
    Log::log(*this, LogMessage::N3005_THROTTLE_X_STOLE_TRAIN_X_FROM_THROTTLE_X, name.value(), train->name.value(), stoleFrom);
  }
  else
  {
    Log::log(*this, LogMessage::I3001_THROTTLE_X_ACQUIRED_TRAIN_X, name.value(), train->name.value());
  }

  Attributes::setEnabled({emergencyStop, throttle, stop, setDirection}, true);

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

  if(m_decoder)
  {
    m_decoder->release(*this);
    m_decoder.reset();
  }
  else if(train)
  {
    const auto logMessage = !stopIt && !train->isStopped.value() ? LogMessage::N3006_THROTTLE_X_RELEASED_TRAIN_X_WITHOUT_STOPPING_IT : LogMessage::I3002_THROTTLE_X_RELEASED_TRAIN_X;
    Log::log(*this, logMessage, name.value(), train->name.value());
    train->release(*this);
    train.setValueInternal(nullptr);
  }

  Attributes::setEnabled({emergencyStop, throttle, stop, setDirection}, false);

  released();
}

void Throttle::destroying()
{
  m_world.throttles->removeObject(shared_ptr<Throttle>());
  release();
  IdObject::destroying();
}

void Throttle::addToWorld()
{
  IdObject::addToWorld();
  m_world.throttles->addObject(shared_ptr<Throttle>());
}

Throttle::AcquireResult Throttle::acquire(std::shared_ptr<Decoder> decoder, bool steal)
{
  if(!decoder->acquire(*this, steal))
    return AcquireResult::FailedInUse;

  m_decoder = std::move(decoder);

  Attributes::setEnabled({emergencyStop, throttle, stop, setDirection}, true);

  return AcquireResult::Success;
}
