/**
 * server/src/hardware/decoder/decoder.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022 Reinder Feenstra
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

#include "decoder.hpp"
#include "list/decoderlist.hpp"
#include "list/decoderlisttablemodel.hpp"
#include "decoderchangeflags.hpp"
#include "decoderfunction.hpp"
#include "decoderfunctions.hpp"
#include "../protocol/dcc/dcc.hpp"
#include "../throttle/throttle.hpp"
#include "../../world/world.hpp"
#include "../../core/objectproperty.tpp"
#include "../../core/attributes.hpp"
#include "../../log/log.hpp"
#include "../../utils/displayname.hpp"
#include "../../utils/almostzero.hpp"

const std::shared_ptr<Decoder> Decoder::null;

Decoder::Decoder(World& world, std::string_view _id) :
  IdObject(world, _id),
  name{this, "name", "", PropertyFlags::ReadWrite | PropertyFlags::Store},
  interface{this, "interface", nullptr, PropertyFlags::ReadWrite | PropertyFlags::Store, nullptr,
    [this](const std::shared_ptr<DecoderController>& newValue)
    {
      if(!newValue || newValue->addDecoder(*this))
      {
        if(interface.value())
          return interface->removeDecoder(*this);
        return true;
      }
      return false;
    }},
  protocol{this, "protocol", DecoderProtocol::Auto, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](const DecoderProtocol& value)
    {
      if(value == DecoderProtocol::DCC && DCC::isLongAddress(address))
        longAddress = true;
      updateEditable();
    }},
  address{this, "address", 0, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](const uint16_t& value)
    {
      if(protocol == DecoderProtocol::DCC)
      {
        if(DCC::isLongAddress(value))
          longAddress = true;
        updateEditable();
      }
    }},
  longAddress{this, "long_address", false, PropertyFlags::ReadWrite | PropertyFlags::Store},
  emergencyStop{this, "emergency_stop", false, PropertyFlags::ReadWrite,
    [this](const bool& /*value*/)
    {
      changed(DecoderChangeFlags::EmergencyStop);
      updateEditable();
    }},
  direction{this, "direction", Direction::Forward, PropertyFlags::ReadWrite,
    [this](const Direction& /*value*/)
    {
      changed(DecoderChangeFlags::Direction);
    },
    [](Direction& value)
    {
      return value != Direction::Unknown;
    }},
  toggleDirection{*this, "toggle_direction",
    [this]()
    {
      direction = (direction == Direction::Forward) ? Direction::Reverse : Direction::Forward;
    }},
  speedSteps{this, "speed_steps", speedStepsAuto, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](const uint8_t& /*value*/)
    {
      changed(DecoderChangeFlags::SpeedSteps);
    }},
  throttle{this, "throttle", throttleMin, PropertyFlags::ReadWrite,
    [this](const double& /*value*/)
    {
      changed(DecoderChangeFlags::Throttle);
      updateEditable();
    }},
  functions{this, "functions", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject},
  notes{this, "notes", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  functions.setValueInternal(std::make_shared<DecoderFunctions>(*this, functions.name()));

  m_worldMute = contains(m_world.state.value(), WorldState::Mute);
  m_worldNoSmoke = contains(m_world.state.value(), WorldState::NoSmoke);

  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, false);
  m_interfaceItems.add(name);

  Attributes::addDisplayName(interface, DisplayName::Hardware::interface);
  Attributes::addEnabled(interface, false);
  Attributes::addObjectList(interface, m_world.decoderControllers);
  m_interfaceItems.add(interface);

  Attributes::addEnabled(protocol, false);
  Attributes::addValues(protocol, decoderProtocolValues);
  m_interfaceItems.add(protocol);

  Attributes::addDisplayName(address, DisplayName::Hardware::address);
  Attributes::addEnabled(address, false);
  m_interfaceItems.add(address);

  Attributes::addEnabled(longAddress, false);
  m_interfaceItems.add(longAddress);
  Attributes::addObjectEditor(emergencyStop, false);
  m_interfaceItems.add(emergencyStop);

  Attributes::addValues(direction, DirectionValues);
  Attributes::addObjectEditor(direction, false);
  m_interfaceItems.add(direction);

  Attributes::addObjectEditor(toggleDirection, false);
  m_interfaceItems.add(toggleDirection);

  Attributes::addDisplayName(speedSteps, DisplayName::Hardware::speedSteps);
  Attributes::addEnabled(speedSteps, false);
  m_interfaceItems.add(speedSteps);

  Attributes::addMinMax(throttle, throttleMin, throttleMax);
  Attributes::addObjectEditor(throttle, false);
  m_interfaceItems.add(throttle);
  m_interfaceItems.add(functions);
  Attributes::addDisplayName(notes, DisplayName::Object::notes);
  m_interfaceItems.add(notes);

  updateEditable();
}

void Decoder::addToWorld()
{
  IdObject::addToWorld();

  m_world.decoders->addObject(shared_ptr<Decoder>());
}

void Decoder::loaded()
{
  IdObject::loaded();
  if(interface)
  {
    if(!interface->addDecoder(*this))
    {
      if(auto object = std::dynamic_pointer_cast<Object>(interface.value()))
        Log::log(*this, LogMessage::C2001_ADDRESS_ALREADY_USED_AT_X, *object);
      interface.setValueInternal(nullptr);
    }
  }
}

bool Decoder::hasFunction(uint32_t number) const
{
  for(const auto& f : *functions)
    if(f->number == number)
      return true;
  return false;
}

std::shared_ptr<const DecoderFunction> Decoder::getFunction(uint32_t number) const
{
  for(const auto& f : *functions)
    if(f->number == number)
      return f;

  return {};
}

const std::shared_ptr<DecoderFunction>& Decoder::getFunction(uint32_t number)
{
  for(const auto& f : *functions)
    if(f->number == number)
      return f;

  return DecoderFunction::null;
}

std::shared_ptr<const DecoderFunction> Decoder::getFunction(DecoderFunctionFunction function) const
{
  for(const auto& f : *functions)
    if(f->function == function)
      return f;

  return {};
}

const std::shared_ptr<DecoderFunction>& Decoder::getFunction(DecoderFunctionFunction function)
{
  for(const auto& f : *functions)
    if(f->function == function)
      return f;

  return DecoderFunction::null;
}

bool Decoder::getFunctionValue(uint32_t number) const
{
  return getFunctionValue(getFunction(number));
}

bool Decoder::getFunctionValue(const std::shared_ptr<const DecoderFunction>& function) const
{
  if(!function)
    return false;

  assert(this == &function->decoder());

  // Apply mute/noSmoke world states:
  if(m_worldMute)
  {
    if(function->function == DecoderFunctionFunction::Mute)
      return true;
    if(function->function == DecoderFunctionFunction::Sound && !getFunction(DecoderFunctionFunction::Mute))
      return false;
  }
  if(m_worldNoSmoke)
  {
    if(function->function == DecoderFunctionFunction::Smoke)
      return false;
  }

  return function->value;
}

void Decoder::setFunctionValue(uint32_t number, bool value)
{
  const auto& f = getFunction(number);
  if(f && getFunctionValue(f) != value)
    f->value.setValueInternal(value);
}

bool Decoder::acquire(Throttle& driver, bool steal)
{
  if(m_driver)
  {
    if(!steal)
      return false;

    m_driver->release();
  }

  assert(!m_driver);

  m_driver = driver.shared_ptr<Throttle>();

  return true;
}

void Decoder::release(Throttle& driver)
{
  if(m_driver.get() == &driver)
    m_driver.reset();
  else
    assert(false);
}

void Decoder::destroying()
{
  if(m_driver) // release driver throttle
  {
    m_driver->release();
    assert(!m_driver);
  }
  if(interface.value())
    interface = nullptr;
  m_world.decoders->removeObject(shared_ptr<Decoder>());
  IdObject::destroying();
}

void Decoder::worldEvent(WorldState state, WorldEvent event)
{
  IdObject::worldEvent(state, event);
  updateEditable(contains(state, WorldState::Edit));

  // Handle mute/noSmoke world states:
  m_worldMute = contains(state, WorldState::Mute);
  m_worldNoSmoke = contains(state, WorldState::NoSmoke);

  if(event == WorldEvent::Mute || event == WorldEvent::Unmute)
  {
    bool hasMute = false;

    for(const auto& f : *functions)
      if(f->function == DecoderFunctionFunction::Mute)
      {
        if(!f->value)
          changed(DecoderChangeFlags::FunctionValue, f->number);
        hasMute = true;
      }

    if(!hasMute)
    {
      for(const auto& f : *functions)
        if(f->function == DecoderFunctionFunction::Sound && f->value)
          changed(DecoderChangeFlags::FunctionValue, f->number);
    }
  }
  else if(event == WorldEvent::NoSmoke || event == WorldEvent::Smoke)
  {
    for(const auto& f : *functions)
      if(f->function == DecoderFunctionFunction::Smoke && f->value)
        changed(DecoderChangeFlags::FunctionValue, f->number);
  }
}

void Decoder::updateEditable()
{
  updateEditable(contains(m_world.state.value(), WorldState::Edit));
}

void Decoder::updateEditable(bool editable)
{
  const bool stopped = editable && almostZero(throttle.value());
  Attributes::setEnabled(name, editable);
  Attributes::setEnabled(interface, stopped);
  Attributes::setEnabled(protocol, stopped);
  Attributes::setEnabled(address, stopped);
  Attributes::setEnabled(longAddress, stopped && protocol == DecoderProtocol::DCC && !DCC::isLongAddress(address));
  Attributes::setEnabled(speedSteps, stopped);
}

void Decoder::changed(DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(interface)
    interface->decoderChanged(*this, changes, functionNumber);
  decoderChanged(*this, changes, functionNumber);
}
