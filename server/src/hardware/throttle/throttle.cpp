/**
 * server/src/hardware/throttle/throttle.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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
#include "../../core/attributes.hpp"
#include "../../core/method.tpp"
#include "../../core/objectproperty.tpp"
#include "../../core/objectvectorproperty.tpp"
#include "../../hardware/decoder/decoder.hpp"
#include "../../utils/displayname.hpp"
#include "../../world/world.hpp"

Throttle::Throttle(World& world, std::string_view _id)
  : IdObject(world, _id)
  , name{this, "name", id, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , emergencyStop{this, "emergency_stop", false, PropertyFlags::ReadWrite,
      [this](const bool& value)
      {
        if(m_decoder)
          m_decoder->emergencyStop = value;
      },
      [this](bool& /*value*/)
      {
        return m_decoder.operator bool();
      }}
  , direction{this, "direction", Direction::Forward, PropertyFlags::ReadWrite,
      [this](const Direction& value)
      {
        if(m_decoder)
          m_decoder->direction = value;
      },
      [this](Direction& value) -> bool
      {
        return m_decoder && value != Direction::Unknown;
      }}
  , throttle{this, "throttle", throttleMin, PropertyFlags::ReadWrite,
      [this](const float& value)
      {
        if(m_decoder)
          m_decoder->throttle = value;
      }}
  , functions{*this, "functions", {}, PropertyFlags::ReadOnly}
{
  const bool editable = contains(m_world.state.value(), WorldState::Edit);

  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);

  Attributes::addEnabled(emergencyStop, false);
  Attributes::addObjectEditor(emergencyStop, false);
  m_interfaceItems.add(emergencyStop);

  Attributes::addEnabled(direction, false);
  Attributes::addValues(direction, DirectionValues);
  Attributes::addObjectEditor(direction, false);
  m_interfaceItems.add(direction);

  Attributes::addEnabled(throttle, false);
  Attributes::addMinMax(throttle, throttleMin, throttleMax);
  Attributes::addObjectEditor(throttle, false);
  m_interfaceItems.add(throttle);
}

#ifndef NDEBUG
Throttle::~Throttle()
{
  assert(!acquired());
}
#endif

bool Throttle::acquired() const
{
  return m_decoder.operator bool();
}

void Throttle::release(bool stop)
{
  if(!acquired())
    return;

  if(stop)
  {
    emergencyStop = true;
    throttle = throttleStop;
  }

  m_decoder->release(*this);
  m_decoder.reset();

  Attributes::setEnabled({emergencyStop, direction, throttle}, false);

  released();
}

void Throttle::destroying()
{
  release();
  IdObject::destroying();
}

Throttle::AcquireResult Throttle::acquire(std::shared_ptr<Decoder> decoder, bool steal)
{
  if(!decoder->acquire(*this, steal))
    return AcquireResult::FailedInUse;

  m_decoder = std::move(decoder);

  for(auto function : *m_decoder->functions)
  {
    const auto& type = function->type.value();
    if(type != DecoderFunctionType::AlwaysOff && type != DecoderFunctionType::AlwaysOn)
    {
      functions.appendInternal(std::make_shared<ThrottleFunction>(*this, function->number));
    }
  }

  Attributes::setEnabled({emergencyStop, direction, throttle}, true);

  return AcquireResult::Success;
}

const std::shared_ptr<ThrottleFunction>& Throttle::getFunction(uint32_t number) const
{
  static const std::shared_ptr<ThrottleFunction> noFunction;

  for(const auto& function : *functions)
    if(function->number == number)
      return function;

  return noFunction;
}

