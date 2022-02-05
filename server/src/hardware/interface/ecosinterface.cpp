/**
 * server/src/hardware/interface/ecosinterface.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
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

#include "ecosinterface.hpp"
#include "../input/list/inputlisttablemodel.hpp"
#include "../output/list/outputlisttablemodel.hpp"
#include "../protocol/ecos/iohandler/tcpiohandler.hpp"
#include "../../core/attributes.hpp"
#include "../../log/log.hpp"
#include "../../log/logmessageexception.hpp"
#include "../../utils/displayname.hpp"
#include "../../utils/inrange.hpp"
#include "../../world/world.hpp"

constexpr auto outputListColumns = OutputListColumn::Id | OutputListColumn::Name | OutputListColumn::Address;

ECoSInterface::ECoSInterface(World& world, std::string_view _id)
  : Interface(world, _id)
  , hostname{this, "hostname", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , ecos{this, "ecos", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
  , decoders{this, "decoders", nullptr, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::SubObject}
  , inputs{this, "inputs", nullptr, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::SubObject}
  , outputs{this, "outputs", nullptr, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::SubObject}
  , testCommand{this, "test_command", "", PropertyFlags::ReadWrite | PropertyFlags::NoStore}
  , testCommandSend{*this, "test_command_send",
    [this]()
    {
      if(m_kernel)
        m_kernel->postSend(testCommand.value() + "\n");
    }}
{
  name = "ECoS";
  ecos.setValueInternal(std::make_shared<ECoS::Settings>(*this, ecos.name()));
  decoders.setValueInternal(std::make_shared<DecoderList>(*this, decoders.name()));
  inputs.setValueInternal(std::make_shared<InputList>(*this, inputs.name()));
  outputs.setValueInternal(std::make_shared<OutputList>(*this, outputs.name(), outputListColumns));

  Attributes::addDisplayName(hostname, DisplayName::IP::hostname);
  Attributes::addEnabled(hostname, !online);
  m_interfaceItems.insertBefore(hostname, notes);

  m_interfaceItems.insertBefore(ecos, notes);

  Attributes::addDisplayName(decoders, DisplayName::Hardware::decoders);
  m_interfaceItems.insertBefore(decoders, notes);

  Attributes::addDisplayName(inputs, DisplayName::Hardware::inputs);
  m_interfaceItems.insertBefore(inputs, notes);

  Attributes::addDisplayName(outputs, DisplayName::Hardware::outputs);
  m_interfaceItems.insertBefore(outputs, notes);

  m_interfaceItems.add(testCommand);
  m_interfaceItems.add(testCommandSend);
}

bool ECoSInterface::addDecoder(Decoder& decoder)
{
  const bool success = DecoderController::addDecoder(decoder);
  if(success)
    decoders->addObject(decoder.shared_ptr<Decoder>());
  return success;
}

bool ECoSInterface::removeDecoder(Decoder& decoder)
{
  const bool success = DecoderController::removeDecoder(decoder);
  if(success)
    decoders->removeObject(decoder.shared_ptr<Decoder>());
  return success;
}

void ECoSInterface::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(m_kernel)
    m_kernel->decoderChanged(decoder, changes, functionNumber);
}

bool ECoSInterface::addInput(Input& input)
{
  const bool success = InputController::addInput(input);
  if(success)
    inputs->addObject(input.shared_ptr<Input>());
  return success;
}

bool ECoSInterface::removeInput(Input& input)
{
  const bool success = InputController::removeInput(input);
  if(success)
    inputs->removeObject(input.shared_ptr<Input>());
  return success;
}

bool ECoSInterface::addOutput(Output& output)
{
  const bool success = OutputController::addOutput(output);
  if(success)
    outputs->addObject(output.shared_ptr<Output>());
  return success;
}

bool ECoSInterface::removeOutput(Output& output)
{
  const bool success = OutputController::removeOutput(output);
  if(success)
    outputs->removeObject(output.shared_ptr<Output>());
  return success;
}

bool ECoSInterface::setOutputValue(uint32_t channel, uint32_t address, bool value)
{
  return
    m_kernel &&
    inRange(address, outputAddressMinMax(channel)) &&
    m_kernel->setOutput(static_cast<uint16_t>(address), value);
}

bool ECoSInterface::setOnline(bool& value)
{
  if(!m_kernel && value)
  {
    try
    {
      m_kernel = ECoS::Kernel::create<ECoS::TCPIOHandler>(ecos->config(), hostname.value());

      status.setValueInternal(InterfaceStatus::Initializing);

      m_kernel->setLogId(id.value());
      m_kernel->setOnStarted(
        [this]()
        {
          status.setValueInternal(InterfaceStatus::Online);
        });

      m_kernel->setDecoderController(this);
      m_kernel->setInputController(this);
      m_kernel->setOutputController(this);
      m_kernel->start();

      m_ecosPropertyChanged = ecos->propertyChanged.connect(
        [this](BaseProperty& /*property*/)
        {
          m_kernel->setConfig(ecos->config());
        });

      if(contains(m_world.state.value(), WorldState::Run))
        m_kernel->go();
      else
        m_kernel->emergencyStop();

      Attributes::setEnabled(hostname, false);
    }
    catch(const LogMessageException& e)
    {
      status.setValueInternal(InterfaceStatus::Offline);
      Log::log(*this, e.message(), e.args());
      return false;
    }
  }
  else if(m_kernel && !value)
  {
    Attributes::setEnabled(hostname, true);

    m_ecosPropertyChanged.disconnect();

    m_kernel->stop();
    m_kernel.reset();

    status.setValueInternal(InterfaceStatus::Offline);
  }
  return true;
}

void ECoSInterface::addToWorld()
{
  Interface::addToWorld();

  m_world.decoderControllers->add(std::dynamic_pointer_cast<DecoderController>(shared_from_this()));
  m_world.inputControllers->add(std::dynamic_pointer_cast<InputController>(shared_from_this()));
  m_world.outputControllers->add(std::dynamic_pointer_cast<OutputController>(shared_from_this()));
}

void ECoSInterface::destroying()
{
  for(const auto& decoder : *decoders)
  {
    assert(decoder->interface.value() == std::dynamic_pointer_cast<DecoderController>(shared_from_this()));
    decoder->interface = nullptr;
  }

  for(const auto& input : *inputs)
  {
    assert(input->interface.value() == std::dynamic_pointer_cast<InputController>(shared_from_this()));
    input->interface = nullptr;
  }

  for(const auto& output : *outputs)
  {
    assert(output->interface.value() == std::dynamic_pointer_cast<OutputController>(shared_from_this()));
    output->interface = nullptr;
  }

  m_world.decoderControllers->remove(std::dynamic_pointer_cast<DecoderController>(shared_from_this()));
  m_world.inputControllers->remove(std::dynamic_pointer_cast<InputController>(shared_from_this()));
  m_world.outputControllers->remove(std::dynamic_pointer_cast<OutputController>(shared_from_this()));

  Interface::destroying();
}

void ECoSInterface::worldEvent(WorldState state, WorldEvent event)
{
  Interface::worldEvent(state, event);

  if(m_kernel)
  {
    switch(event)
    {
      case WorldEvent::PowerOff:
      case WorldEvent::Stop:
        m_kernel->emergencyStop();
        break;

      case WorldEvent::PowerOn:
      case WorldEvent::Run:
        if(contains(state, WorldState::PowerOn | WorldState::Run))
          m_kernel->go();
        break;

      default:
        break;
    }
  }
}

void ECoSInterface::idChanged(const std::string& newId)
{
  if(m_kernel)
    m_kernel->setLogId(newId);
}
