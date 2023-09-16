/**
 * server/src/hardware/decoder/decoder.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2023 Reinder Feenstra
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
#include "../../core/method.tpp"
#include "../../log/log.hpp"
#include "../../utils/displayname.hpp"
#include "../../utils/almostzero.hpp"

CREATE_IMPL(Decoder)

const std::shared_ptr<Decoder> Decoder::null;

Decoder::Decoder(World& world, std::string_view _id) :
  IdObject(world, _id),
  name{this, "name", "", PropertyFlags::ReadWrite | PropertyFlags::Store},
  interface{this, "interface", nullptr, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](const std::shared_ptr<DecoderController>& value)
    {
      if(value)
      {
        assert(!value->decoderProtocols().empty());
        Attributes::setValues(protocol, value->decoderProtocols());
        if(!checkProtocol())
          protocolChanged();
        Attributes::setVisible(protocol, true);
      }
      else
        Attributes::setVisible(protocol, false);
      updateEditable();
    },
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
  protocol{this, "protocol", DecoderProtocol::None, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](const DecoderProtocol& /*value*/)
    {
      protocolChanged();
      updateEditable();
    }},
  address{this, "address", 0, PropertyFlags::ReadWrite | PropertyFlags::Store},
  mfxUID{this, "mfx_uid", 0, PropertyFlags::ReadWrite | PropertyFlags::Store},
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
  Attributes::addValues(protocol, tcb::span<const DecoderProtocol>{});
  Attributes::addVisible(protocol, false);
  m_interfaceItems.add(protocol);

  Attributes::addDisplayName(address, DisplayName::Hardware::address);
  Attributes::addEnabled(address, false);
  Attributes::addMinMax(address, std::pair<uint16_t, uint16_t>(0, 0));
  Attributes::addVisible(address, false);
  m_interfaceItems.add(address);

  Attributes::addEnabled(mfxUID, false);
  Attributes::addVisible(mfxUID, false);
  m_interfaceItems.add(mfxUID);

  Attributes::addObjectEditor(emergencyStop, false);
  m_interfaceItems.add(emergencyStop);

  Attributes::addValues(direction, DirectionValues);
  Attributes::addObjectEditor(direction, false);
  m_interfaceItems.add(direction);

  Attributes::addObjectEditor(toggleDirection, false);
  m_interfaceItems.add(toggleDirection);

  Attributes::addDisplayName(speedSteps, DisplayName::Hardware::speedSteps);
  Attributes::addEnabled(speedSteps, false);
  Attributes::addValues(speedSteps, tcb::span<const uint8_t>{});
  Attributes::addVisible(speedSteps, false);
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
    Attributes::setValues(protocol, interface->decoderProtocols());
    Attributes::setVisible(protocol, true);
    checkProtocol(); //! \todo log something if protocol is changed??
    protocolChanged();

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

void Decoder::protocolChanged()
{
  if(interface)
  {
    // address:
    const auto addressRange = interface->decoderAddressMinMax(protocol);
    const bool hasAddress = addressRange.first <= addressRange.second;
    Attributes::setVisible(address, hasAddress);
    if(hasAddress)
    {
      Attributes::setMinMax(address, addressRange);
      checkAddress();
    }
    else
      Attributes::setMinMax(address, std::pair<uint16_t, uint16_t>(0, 0));

    if(protocol == DecoderProtocol::MFX)
      address = 0;

    // MFX:
    Attributes::setVisible(mfxUID, protocol == DecoderProtocol::MFX);

    // speed steps:
    const auto values = interface->decoderSpeedSteps(protocol);
    Attributes::setVisible(speedSteps, !values.empty());
    if(!values.empty())
    {
      Attributes::setValues(speedSteps, values);
      checkSpeedSteps();
    }
    else
      Attributes::setValues(speedSteps, tcb::span<const uint8_t>{});
  }
  else
  {
    Attributes::setVisible(address, false);
    Attributes::setVisible(mfxUID, false);
    Attributes::setVisible(speedSteps, false);
  }
}

bool Decoder::checkProtocol()
{
  const auto protocols = protocol.getSpanAttribute<DecoderProtocol>(AttributeName::Values).values();
  assert(!protocols.empty());
  if(auto it = std::find(protocols.begin(), protocols.end(), protocol); it == protocols.end())
  {
    protocol = protocols.front();
    return true;
  }
  return false;
}

bool Decoder::checkAddress()
{
  const auto addressMin = address.getAttribute<uint16_t>(AttributeName::Min);
  const auto addressMax = address.getAttribute<uint16_t>(AttributeName::Max);
  if(!inRange(address.value(), addressMin, addressMax))
  {
    address = std::clamp(address.value(), addressMin, addressMax);
    return true;
  }
  return false;
}

bool Decoder::checkSpeedSteps()
{
  const auto values = speedSteps.getSpanAttribute<uint8_t>(AttributeName::Values).values();
  if(auto it = std::find(values.begin(), values.end(), speedSteps); it == values.end())
  {
    speedSteps = values.back();
    return true;
  }
  return false;
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
  Attributes::setEnabled(protocol, stopped && protocol.getSpanAttribute<DecoderProtocol>(AttributeName::Values).length() > 1);
  Attributes::setEnabled(address, stopped);
  Attributes::setEnabled(speedSteps, stopped && speedSteps.getSpanAttribute<uint8_t>(AttributeName::Values).length() > 1);
}

void Decoder::changed(DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(interface)
    interface->decoderChanged(*this, changes, functionNumber);
  decoderChanged(*this, changes, functionNumber);
}
