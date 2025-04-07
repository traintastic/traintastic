/**
 * server/src/hardware/interface/z21interface.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2025 Reinder Feenstra
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
#include "../decoder/list/decoderlist.hpp"
#include "../input/list/inputlist.hpp"
#include "../output/list/outputlist.hpp"
#include "../protocol/dcc/dcc.hpp"
#include "../protocol/z21/clientkernel.hpp"
#include "../protocol/z21/clientsettings.hpp"
#include "../protocol/z21/messages.hpp"
#include "../protocol/z21/iohandler/simulationiohandler.hpp"
#include "../protocol/z21/iohandler/udpclientiohandler.hpp"
#include "../../core/attributes.hpp"
#include "../../core/eventloop.hpp"
#include "../../core/method.tpp"
#include "../../core/objectproperty.tpp"
#include "../../log/log.hpp"
#include "../../log/logmessageexception.hpp"
#include "../../utils/category.hpp"
#include "../../utils/displayname.hpp"
#include "../../utils/inrange.hpp"
#include "../../utils/makearray.hpp"
#include "../../world/world.hpp"

constexpr auto decoderListColumns = DecoderListColumn::Id | DecoderListColumn::Name | DecoderListColumn::Protocol | DecoderListColumn::Address;
constexpr auto inputListColumns = InputListColumn::Id | InputListColumn::Name | InputListColumn::Channel | InputListColumn::Address;
constexpr auto outputListColumns = OutputListColumn::Channel | OutputListColumn::Address;

CREATE_IMPL(Z21Interface)

Z21Interface::Z21Interface(World& world, std::string_view _id)
  : Interface(world, _id)
  , DecoderController(*this, decoderListColumns)
  , InputController(static_cast<IdObject&>(*this))
  , OutputController(static_cast<IdObject&>(*this))
  , hostname{this, "hostname", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , port{this, "port", 21105, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , z21{this, "z21", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
  , hardwareType{this, "hardware_type", "", PropertyFlags::ReadOnly | PropertyFlags::NoStore}
  , serialNumber{this, "serial_number", "", PropertyFlags::ReadOnly | PropertyFlags::NoStore}
  , firmwareVersion{this, "firmware_version", "", PropertyFlags::ReadOnly | PropertyFlags::NoStore}
{
  name = "Z21";
  z21.setValueInternal(std::make_shared<Z21::ClientSettings>(*this, z21.name()));

  Attributes::addDisplayName(hostname, DisplayName::IP::hostname);
  Attributes::addEnabled(hostname, !online);
  m_interfaceItems.insertBefore(hostname, notes);

  Attributes::addDisplayName(port, DisplayName::IP::port);
  Attributes::addEnabled(port, !online);
  m_interfaceItems.insertBefore(port, notes);

  Attributes::addDisplayName(z21, DisplayName::Hardware::z21);
  m_interfaceItems.insertBefore(z21, notes);

  m_interfaceItems.insertBefore(decoders, notes);

  m_interfaceItems.insertBefore(inputs, notes);

  m_interfaceItems.insertBefore(outputs, notes);

  Attributes::addCategory(hardwareType, Category::info);
  m_interfaceItems.insertBefore(hardwareType, notes);

  Attributes::addCategory(serialNumber, Category::info);
  m_interfaceItems.insertBefore(serialNumber, notes);

  Attributes::addCategory(firmwareVersion, Category::info);
  m_interfaceItems.insertBefore(firmwareVersion, notes);
}

std::span<const DecoderProtocol> Z21Interface::decoderProtocols() const
{
  static constexpr std::array<DecoderProtocol, 3> protocols{DecoderProtocol::DCCShort, DecoderProtocol::DCCLong, DecoderProtocol::Motorola};
  return std::span<const DecoderProtocol>{protocols.data(), protocols.size()};
}

std::pair<uint16_t, uint16_t> Z21Interface::decoderAddressMinMax(DecoderProtocol protocol) const
{
  if(protocol == DecoderProtocol::DCCLong)
  {
    return {DCC::addressLongStart, DCC::addressLongMax};
  }
  return DecoderController::decoderAddressMinMax(protocol);
}

void Z21Interface::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(m_kernel)
    m_kernel->decoderChanged(decoder, changes, functionNumber);
}

const std::vector<uint32_t> *Z21Interface::inputChannels() const
{
  return &Z21::ClientKernel::inputChannels;
}

const std::vector<std::string_view> *Z21Interface::inputChannelNames() const
{
  return &Z21::ClientKernel::inputChannelNames;
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

void Z21Interface::inputSimulateChange(uint32_t channel, uint32_t address, SimulateInputAction action)
{
  if(m_kernel && inRange(address, inputAddressMinMax(channel)))
    m_kernel->simulateInputChange(channel, address, action);
}

std::span<const OutputChannel> Z21Interface::outputChannels() const
{
  static const auto values = makeArray(OutputChannel::Accessory, OutputChannel::DCCext);
  return values;
}

std::pair<uint32_t, uint32_t> Z21Interface::outputAddressMinMax(OutputChannel channel) const
{
  if(channel == OutputChannel::Accessory)
  {
    return {Z21::ClientKernel::outputAddressMin, Z21::ClientKernel::outputAddressMax};
  }
  return OutputController::outputAddressMinMax(channel);
}

bool Z21Interface::setOutputValue(OutputChannel channel, uint32_t address, OutputValue value)
{
  return
      m_kernel &&
      inRange(address, outputAddressMinMax(channel)) &&
      m_kernel->setOutput(channel, static_cast<uint16_t>(address), value);
}

bool Z21Interface::setOnline(bool& value, bool simulation)
{
  if(!m_kernel && value)
  {
    try
    {
      if(simulation)
        m_kernel = Z21::ClientKernel::create<Z21::SimulationIOHandler>(id.value(), z21->config());
      else
        m_kernel = Z21::ClientKernel::create<Z21::UDPClientIOHandler>(id.value(), z21->config(), hostname.value(), port.value());

      setState(InterfaceState::Initializing);

      m_kernel->setOnStarted(
        [this]()
        {
          setState(InterfaceState::Online);
        });
      m_kernel->setOnError(
        [this]()
        {
          setState(InterfaceState::Error);
          online = false; // communication no longer possible
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
      m_kernel->setOnTrackPowerChanged(
        [this](bool powerOn, bool isStopped)
        {
          if(powerOn)
          {
            /* NOTE:
             * Setting stop and powerOn together is not an atomic operation,
             * so it would trigger 2 state changes with in the middle state.
             * Fortunately this does not happen because at least one of the state is already set.
             * Because if we are in Run state we go to PowerOn,
             * and if we are on PowerOff then we go to PowerOn.
             */

            // First of all, stop if we have to, otherwhise we might inappropiately run trains
            if(isStopped && contains(m_world.state.value(), WorldState::Run))
            {
              m_world.stop();
            }
            else if(!contains(m_world.state.value(), WorldState::Run) && !isStopped)
            {
              m_world.run(); // Run trains yay!
            }

            // EmergencyStop in Z21 also means power is still on
            if(!contains(m_world.state.value(), WorldState::PowerOn) && isStopped)
            {
              m_world.powerOn(); // Just power on but keep stopped
            }
          }
          else
          {
            // Power off regardless of stop state
            if(contains(m_world.state.value(), WorldState::PowerOn))
              m_world.powerOff();
          }
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

      // Avoid to set multiple power states in rapid succession
      if(contains(m_world.state.value(), WorldState::PowerOn))
      {
        if(contains(m_world.state.value(), WorldState::Run))
          m_kernel->trackPowerOn(); // Run trains
        else
          m_kernel->emergencyStop(); // Emergency stop with power on
      }
      else
      {
        m_kernel->trackPowerOff(); // Stop by powering off
      }

      Attributes::setEnabled({hostname, port}, false);
    }
    catch(const LogMessageException& e)
    {
      setState(InterfaceState::Offline);
      Log::log(*this, e.message(), e.args());
      return false;
    }
  }
  else if(m_kernel && !value)
  {
    Attributes::setEnabled({hostname, port}, true);

    m_z21PropertyChanged.disconnect();

    m_kernel->stop();
    EventLoop::deleteLater(m_kernel.release());

    setState(InterfaceState::Offline);
    hardwareType.setValueInternal("");
    serialNumber.setValueInternal("");
    firmwareVersion.setValueInternal("");
  }
  return true;
}

void Z21Interface::addToWorld()
{
  Interface::addToWorld();
  DecoderController::addToWorld();
  InputController::addToWorld(inputListColumns);
  OutputController::addToWorld(outputListColumns);
}

void Z21Interface::destroying()
{
  OutputController::destroying();
  InputController::destroying();
  DecoderController::destroying();
  Interface::destroying();
}

void Z21Interface::worldEvent(WorldState state, WorldEvent event)
{
  Interface::worldEvent(state, event);

  if(m_kernel)
  {
    // Avoid to set multiple power states in rapid succession
    switch(event)
    {
      case WorldEvent::PowerOff:
      {
        m_kernel->trackPowerOff();
        break;
      }
      case WorldEvent::PowerOn:
      {
        if(contains(state, WorldState::Run))
          m_kernel->trackPowerOn();
        else
          m_kernel->emergencyStop(); // In Z21 E-Stop means power on but not running
        break;
      }
      case WorldEvent::Stop:
      {
        if(contains(state, WorldState::PowerOn))
        {
          // In Z21 E-Stop means power is on but trains are not running
          m_kernel->emergencyStop();
        }
        else
        {
          // This Stops everything by removing power
          m_kernel->trackPowerOff();
        }
        break;
      }
      case WorldEvent::Run:
      {
        if(contains(state, WorldState::PowerOn))
          m_kernel->trackPowerOn();
        break;
      }
      default:
        break;
    }
  }
}
