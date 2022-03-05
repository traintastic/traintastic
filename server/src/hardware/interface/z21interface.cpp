/**
 * server/src/hardware/interface/z21interface.cpp
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

#include "z21interface.hpp"
#include "../decoder/list/decoderlisttablemodel.hpp"
#include "../protocol/z21/messages.hpp"
#include "../protocol/z21/iohandler/simulationiohandler.hpp"
#include "../protocol/z21/iohandler/udpclientiohandler.hpp"
#include "../../core/attributes.hpp"
#include "../../log/log.hpp"
#include "../../log/logmessageexception.hpp"
#include "../../utils/category.hpp"
#include "../../utils/displayname.hpp"
#include "../../utils/inrange.hpp"
#include "../../world/world.hpp"

constexpr auto decoderListColumns = DecoderListColumn::Id | DecoderListColumn::Name | DecoderListColumn::Address;
constexpr auto inputListColumns = InputListColumn::Id | InputListColumn::Name | InputListColumn::Channel | InputListColumn::Address;
constexpr auto outputListColumns = OutputListColumn::Id | OutputListColumn::Name | OutputListColumn::Address;

Z21Interface::Z21Interface(World& world, std::string_view _id)
  : Interface(world, _id)
  , hostname{this, "hostname", "192.168.1.203", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , port{this, "port", 21105, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , z21{this, "z21", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
  , decoders{this, "decoders", nullptr, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::SubObject}
  , inputs{this, "inputs", nullptr, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::SubObject}
  , outputs{this, "outputs", nullptr, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::SubObject}
  , hardwareType{this, "hardware_type", "", PropertyFlags::ReadOnly | PropertyFlags::NoStore}
  , serialNumber{this, "serial_number", "", PropertyFlags::ReadOnly | PropertyFlags::NoStore}
  , firmwareVersion{this, "firmware_version", "", PropertyFlags::ReadOnly | PropertyFlags::NoStore}
{
  name = "Z21";
  z21.setValueInternal(std::make_shared<Z21::ClientSettings>(*this, z21.name()));
  decoders.setValueInternal(std::make_shared<DecoderList>(*this, decoders.name(), decoderListColumns));
  inputs.setValueInternal(std::make_shared<InputList>(*this, inputs.name(), inputListColumns));
  outputs.setValueInternal(std::make_shared<OutputList>(*this, outputs.name(), outputListColumns));

  Attributes::addDisplayName(hostname, DisplayName::IP::hostname);
  Attributes::addEnabled(hostname, !online);
  m_interfaceItems.insertBefore(hostname, notes);

  Attributes::addDisplayName(port, DisplayName::IP::port);
  Attributes::addEnabled(port, !online);
  m_interfaceItems.insertBefore(port, notes);

  Attributes::addDisplayName(z21, DisplayName::Hardware::z21);
  m_interfaceItems.insertBefore(z21, notes);

  Attributes::addDisplayName(decoders, DisplayName::Hardware::decoders);
  m_interfaceItems.insertBefore(decoders, notes);

  Attributes::addDisplayName(inputs, DisplayName::Hardware::inputs);
  m_interfaceItems.insertBefore(inputs, notes);

  Attributes::addDisplayName(outputs, DisplayName::Hardware::outputs);
  m_interfaceItems.insertBefore(outputs, notes);

  Attributes::addCategory(hardwareType, Category::info);
  m_interfaceItems.insertBefore(hardwareType, notes);

  Attributes::addCategory(serialNumber, Category::info);
  m_interfaceItems.insertBefore(serialNumber, notes);

  Attributes::addCategory(firmwareVersion, Category::info);
  m_interfaceItems.insertBefore(firmwareVersion, notes);
}

bool Z21Interface::addDecoder(Decoder& decoder)
{
  const bool success = DecoderController::addDecoder(decoder);
  if(success)
    decoders->addObject(decoder.shared_ptr<Decoder>());
  return success;
}

bool Z21Interface::removeDecoder(Decoder& decoder)
{
  const bool success = DecoderController::removeDecoder(decoder);
  if(success)
    decoders->removeObject(decoder.shared_ptr<Decoder>());
  return success;
}

void Z21Interface::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(m_kernel)
    m_kernel->decoderChanged(decoder, changes, functionNumber);
}

std::pair<uint32_t, uint32_t> Z21Interface::inputAddressMinMax(uint32_t channel) const
{
  using namespace Z21;

  switch(channel)
  {
    case ClientKernel::InputChannel::rbus:
      return {ClientKernel::rbusAddressMin, ClientKernel::rbusAddressMax};

    case ClientKernel::InputChannel::loconet:
      return {ClientKernel::loconetAddressMin, ClientKernel::loconetAddressMax};
  }

  assert(false);
  return {0, 0};
}

bool Z21Interface::addInput(Input& input)
{
  const bool success = InputController::addInput(input);
  if(success)
    inputs->addObject(input.shared_ptr<Input>());
  return success;
}

bool Z21Interface::removeInput(Input& input)
{
  const bool success = InputController::removeInput(input);
  if(success)
    inputs->removeObject(input.shared_ptr<Input>());
  return success;
}

bool Z21Interface::addOutput(Output& output)
{
  const bool success = OutputController::addOutput(output);
  if(success)
    outputs->addObject(output.shared_ptr<Output>());
  return success;
}

bool Z21Interface::removeOutput(Output& output)
{
  const bool success = OutputController::removeOutput(output);
  if(success)
    outputs->removeObject(output.shared_ptr<Output>());
  return success;
}

bool Z21Interface::setOutputValue(uint32_t channel, uint32_t address, bool value)
{
  return
    m_kernel &&
    inRange(address, outputAddressMinMax(channel)) &&
    m_kernel->setOutput(static_cast<uint16_t>(address), value);
}

bool Z21Interface::setOnline(bool& value, bool simulation)
{
  if(!m_kernel && value)
  {
    try
    {
      if(simulation)
        m_kernel = Z21::ClientKernel::create<Z21::SimulationIOHandler>(z21->config());
      else
        m_kernel = Z21::ClientKernel::create<Z21::UDPClientIOHandler>(z21->config(), hostname.value(), port.value());

      status.setValueInternal(InterfaceStatus::Initializing);

      m_kernel->setLogId(id.value());
      m_kernel->setOnStarted(
        [this]()
        {
          status.setValueInternal(InterfaceStatus::Online);
        });
      m_kernel->setOnSerialNumberChanged(
        [this](uint32_t newValue)
        {
          serialNumber.setValueInternal(std::to_string(newValue));
        });
      m_kernel->setOnHardwareInfoChanged(
        [this](Z21::HardwareType type, uint8_t versionMajor, uint8_t versionMinor)
        {
          hardwareType.setValueInternal(std::string(Z21::toString(type)));
          Log::log(*this, LogMessage::I2002_HARDWARE_TYPE_X, hardwareType.value());

          if(versionMajor != 0 || versionMinor != 0)
          {
            firmwareVersion.setValueInternal(std::to_string(versionMajor).append(".").append(std::to_string(versionMinor)));
            Log::log(*this, LogMessage::I2003_FIRMWARE_VERSION_X, firmwareVersion.value());
          }
          else
            firmwareVersion.setValueInternal("");
        });
      m_kernel->setOnTrackPowerOnChanged(
        [this](bool powerOn)
        {
          if(powerOn == contains(m_world.state.value(), WorldState::PowerOn))
            return;

          if(powerOn)
            m_world.powerOn();
          else
            m_world.powerOff();
        });
      m_kernel->setOnEmergencyStop(
        [this]()
        {
          if(contains(m_world.state.value(), WorldState::Run))
            m_world.stop();
        });

      m_kernel->setDecoderController(this);
      m_kernel->setInputController(this);
      m_kernel->setOutputController(this);

      m_kernel->start();

      m_z21PropertyChanged = z21->propertyChanged.connect(
        [this](BaseProperty& /*property*/)
        {
          m_kernel->setConfig(z21->config());
        });

      if(contains(m_world.state.value(), WorldState::PowerOn))
        m_kernel->trackPowerOn();
      else
        m_kernel->trackPowerOff();

      if(!contains(m_world.state.value(), WorldState::Run))
        m_kernel->emergencyStop();

      Attributes::setEnabled({hostname, port}, false);
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
    Attributes::setEnabled({hostname, port}, true);

    m_z21PropertyChanged.disconnect();

    m_kernel->stop();
    m_kernel.reset();

    status.setValueInternal(InterfaceStatus::Offline);
    hardwareType.setValueInternal("");
    serialNumber.setValueInternal("");
    firmwareVersion.setValueInternal("");
  }
  return true;
}

void Z21Interface::addToWorld()
{
  Interface::addToWorld();

  m_world.decoderControllers->add(std::dynamic_pointer_cast<DecoderController>(shared_from_this()));
  m_world.outputControllers->add(std::dynamic_pointer_cast<OutputController>(shared_from_this()));
}

void Z21Interface::destroying()
{
  for(const auto& decoder : *decoders)
  {
    assert(decoder->interface.value() == std::dynamic_pointer_cast<DecoderController>(shared_from_this()));
    decoder->interface = nullptr;
  }

  for(const auto& output : *outputs)
  {
    assert(output->interface.value() == std::dynamic_pointer_cast<OutputController>(shared_from_this()));
    output->interface = nullptr;
  }

  m_world.decoderControllers->remove(std::dynamic_pointer_cast<DecoderController>(shared_from_this()));
  m_world.outputControllers->remove(std::dynamic_pointer_cast<OutputController>(shared_from_this()));

  Interface::destroying();
}

void Z21Interface::worldEvent(WorldState state, WorldEvent event)
{
  Interface::worldEvent(state, event);

  if(m_kernel)
  {
    switch(event)
    {
      case WorldEvent::PowerOff:
        m_kernel->trackPowerOff();
        break;

      case WorldEvent::PowerOn:
        m_kernel->trackPowerOn();
        if(!contains(state, WorldState::Run))
          m_kernel->emergencyStop();
        break;

      case WorldEvent::Stop:
        m_kernel->emergencyStop();
        break;

      case WorldEvent::Run:
        if(contains(state, WorldState::PowerOn))
          m_kernel->trackPowerOn();
        break;

      default:
        break;
    }
  }
}

void Z21Interface::idChanged(const std::string& newId)
{
  if(m_kernel)
    m_kernel->setLogId(newId);
}
