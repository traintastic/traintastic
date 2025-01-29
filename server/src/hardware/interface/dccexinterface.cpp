/**
 * server/src/hardware/interface/dccexinterface.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2025 Reinder Feenstra
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

#include "dccexinterface.hpp"
#include "../decoder/list/decoderlist.hpp"
#include "../decoder/list/decoderlisttablemodel.hpp"
#include "../input/list/inputlist.hpp"
#include "../output/list/outputlist.hpp"
#include "../protocol/dcc/dcc.hpp"
#include "../protocol/dccex/kernel.hpp"
#include "../protocol/dccex/settings.hpp"
#include "../protocol/dccex/messages.hpp"
#include "../protocol/dccex/iohandler/serialiohandler.hpp"
#include "../protocol/dccex/iohandler/tcpiohandler.hpp"
#include "../protocol/dccex/iohandler/simulationiohandler.hpp"
#include "../../core/attributes.hpp"
#include "../../core/eventloop.hpp"
#include "../../core/method.tpp"
#include "../../core/objectproperty.tpp"
#include "../../log/log.hpp"
#include "../../log/logmessageexception.hpp"
#include "../../utils/displayname.hpp"
#include "../../utils/inrange.hpp"
#include "../../utils/makearray.hpp"
#include "../../utils/serialport.hpp"
#include "../../world/world.hpp"

constexpr auto decoderListColumns = DecoderListColumn::Id | DecoderListColumn::Name | DecoderListColumn::Address;
constexpr auto inputListColumns = InputListColumn::Id | InputListColumn::Name | InputListColumn::Address;
constexpr auto outputListColumns = OutputListColumn::Channel | OutputListColumn::Address;

CREATE_IMPL(DCCEXInterface)

DCCEXInterface::DCCEXInterface(World& world, std::string_view _id)
  : Interface(world, _id)
  , DecoderController(*this, decoderListColumns)
  , InputController(static_cast<IdObject&>(*this))
  , OutputController(static_cast<IdObject&>(*this))
  , type{this, "type", DCCEXInterfaceType::Serial, PropertyFlags::ReadWrite | PropertyFlags::Store,
      [this](DCCEXInterfaceType /*value*/)
      {
        updateVisible();
      }}
  , device{this, "device", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , baudrate{this, "baudrate", 115200, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , hostname{this, "hostname", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , port{this, "port", 2560, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , dccex{this, "dccex", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
{
  name = "DCC-EX";
  dccex.setValueInternal(std::make_shared<DCCEX::Settings>(*this, dccex.name()));

  Attributes::addDisplayName(type, DisplayName::Interface::type);
  Attributes::addEnabled(type, !online);
  Attributes::addValues(type, DCCEXInterfaceTypeValues);
  m_interfaceItems.insertBefore(type, notes);

  Attributes::addEnabled(device, !online);
  Attributes::addVisible(device, false);
  m_interfaceItems.insertBefore(device, notes);

  Attributes::addDisplayName(baudrate, DisplayName::Serial::baudrate);
  Attributes::addEnabled(baudrate, !online);
  Attributes::addVisible(baudrate, false);
  Attributes::addMinMax(baudrate, SerialPort::baudrateMin, SerialPort::baudrateMax);
  Attributes::addValues(baudrate, SerialPort::baudrateValues);
  m_interfaceItems.insertBefore(baudrate, notes);

  Attributes::addDisplayName(hostname, DisplayName::IP::hostname);
  Attributes::addEnabled(hostname, !online);
  Attributes::addVisible(hostname, false);
  m_interfaceItems.insertBefore(hostname, notes);

  Attributes::addDisplayName(port, DisplayName::IP::port);
  Attributes::addEnabled(port, !online);
  Attributes::addVisible(port, false);
  m_interfaceItems.insertBefore(port, notes);

  Attributes::addDisplayName(dccex, DisplayName::Hardware::dccex);
  m_interfaceItems.insertBefore(dccex, notes);

  m_interfaceItems.insertBefore(decoders, notes);

  m_interfaceItems.insertBefore(inputs, notes);

  m_interfaceItems.insertBefore(outputs, notes);

  m_dccexPropertyChanged = dccex->propertyChanged.connect(
    [this](BaseProperty& property)
    {
      if(m_kernel && &property != &dccex->startupDelay)
        m_kernel->setConfig(dccex->config());

      if(&property == &dccex->speedSteps)
      {
        // update speedsteps of all decoders, DCC-EX only has a global speedsteps setting
        const auto values = decoderSpeedSteps(DecoderProtocol::DCCShort); // identical for DCCLong
        assert(values.size() == 1);
        for(const auto& decoder : *decoders)
        {
          Attributes::setValues(decoder->speedSteps, values);
          decoder->speedSteps.setValueInternal(values.front());
        }
      }
    });

  updateEnabled();
  updateVisible();
}

std::span<const DecoderProtocol> DCCEXInterface::decoderProtocols() const
{
  static constexpr std::array<DecoderProtocol, 2> protocols{DecoderProtocol::DCCShort, DecoderProtocol::DCCLong};
  return std::span<const DecoderProtocol>{protocols.data(), protocols.size()};
}

std::pair<uint16_t, uint16_t> DCCEXInterface::decoderAddressMinMax(DecoderProtocol protocol) const
{
  if(protocol == DecoderProtocol::DCCLong)
    return {DCC::addressLongStart, DCC::addressLongMax}; // DCC-EX considers all addresses below 128 as short.
  return DecoderController::decoderAddressMinMax(protocol);
}

std::span<const uint8_t> DCCEXInterface::decoderSpeedSteps(DecoderProtocol protocol) const
{
  (void)protocol; // silence unused warning for release build
  assert(protocol == DecoderProtocol::DCCShort || protocol == DecoderProtocol::DCCLong);
  const auto& speedStepValues = DCCEX::Settings::speedStepValues;
  // find value in array so we can create a span, using a span of a variable won't work due to the compare with prevous value in the attribute setter
  if(const auto it = std::find(speedStepValues.begin(), speedStepValues.end(), dccex->speedSteps); it != speedStepValues.end()) /*[[likely]]/*/ // NOLINT(readability-qualified-auto) windows requires const auto
    return {&(*it), 1};
  assert(false);
  return {};
}

void DCCEXInterface::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(m_kernel)
    m_kernel->decoderChanged(decoder, changes, functionNumber);
}

std::pair<uint32_t, uint32_t> DCCEXInterface::inputAddressMinMax(uint32_t /*channel*/) const
{
  return {DCCEX::Kernel::idMin, DCCEX::Kernel::idMax};
}

void DCCEXInterface::inputSimulateChange(uint32_t channel, uint32_t address, SimulateInputAction action)
{
  if(m_kernel && inRange(address, inputAddressMinMax(channel)))
    m_kernel->simulateInputChange(address, action);
}

std::span<const OutputChannel> DCCEXInterface::outputChannels() const
{
  static const auto values = makeArray(OutputChannel::Accessory, OutputChannel::Turnout, OutputChannel::Output, OutputChannel::DCCext);
  return values;
}

std::pair<uint32_t, uint32_t> DCCEXInterface::outputAddressMinMax(OutputChannel channel) const
{
  using namespace DCCEX;

  switch(channel)
  {
    case OutputChannel::Accessory:
      return OutputController::outputAddressMinMax(OutputChannel::AccessoryDCC);

    case OutputChannel::Turnout:
    case OutputChannel::Output:
      return {Kernel::idMin, Kernel::idMax};

    default:
      return OutputController::outputAddressMinMax(channel);
  }
}

bool DCCEXInterface::setOutputValue(OutputChannel channel, uint32_t address, OutputValue value)
{
  return
    m_kernel &&
    inRange(address, outputAddressMinMax(channel)) &&
    m_kernel->setOutput(channel, static_cast<uint16_t>(address), value);
}

bool DCCEXInterface::setOnline(bool& value, bool simulation)
{
  if(!m_kernel && value)
  {
    try
    {
      if(simulation)
      {
        m_kernel = DCCEX::Kernel::create<DCCEX::SimulationIOHandler>(id.value(), dccex->config());
      }
      else
      {
        switch(type)
        {
          case DCCEXInterfaceType::Serial:
            m_kernel = DCCEX::Kernel::create<DCCEX::SerialIOHandler>(id.value(), dccex->config(), device.value(), baudrate.value(), SerialFlowControl::None);
            break;

          case DCCEXInterfaceType::NetworkTCP:
            m_kernel = DCCEX::Kernel::create<DCCEX::TCPIOHandler>(id.value(), dccex->config(), hostname.value(), port.value());
            break;
        }

      }

      setState(InterfaceState::Initializing);

      m_kernel->setOnStarted(
        [this]()
        {
          setState(InterfaceState::Online);

          const bool powerOn = contains(m_world.state.value(), WorldState::PowerOn);

          if(!powerOn)
            m_kernel->powerOff();

          if(contains(m_world.state.value(), WorldState::Run))
          {
            m_kernel->clearEmergencyStop();
            restoreDecoderSpeed();
          }
          else
            m_kernel->emergencyStop();

          if(powerOn)
            m_kernel->powerOn();
        });
      m_kernel->setOnError(
        [this]()
        {
          setState(InterfaceState::Error);
          online = false; // communication no longer possible
        });
      m_kernel->setOnPowerOnChanged(
        [this](bool powerOn)
        {
          if(powerOn && !contains(m_world.state.value(), WorldState::PowerOn))
            m_world.powerOn();
          else if(!powerOn && contains(m_world.state.value(), WorldState::PowerOn))
            m_world.powerOff();
        });
      m_kernel->setDecoderController(this);
      m_kernel->setInputController(this);
      m_kernel->setOutputController(this);
      m_kernel->start();

      Attributes::setEnabled({type, device, baudrate, hostname, port}, false);
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
    Attributes::setEnabled({type, device, baudrate, hostname, port}, true);

    m_kernel->stop();
    EventLoop::deleteLater(m_kernel.release());

    if(status->state != InterfaceState::Error)
    {
      setState(InterfaceState::Offline);
    }
  }
  return true;
}

void DCCEXInterface::addToWorld()
{
  Interface::addToWorld();
  DecoderController::addToWorld();
  InputController::addToWorld(inputListColumns);
  OutputController::addToWorld(outputListColumns);
}

void DCCEXInterface::loaded()
{
  Interface::loaded();

  check();
  updateEnabled();
  updateVisible();
}

void DCCEXInterface::destroying()
{
  m_dccexPropertyChanged.disconnect();
  OutputController::destroying();
  InputController::destroying();
  DecoderController::destroying();
  Interface::destroying();
}

void DCCEXInterface::worldEvent(WorldState state, WorldEvent event)
{
  Interface::worldEvent(state, event);

  switch(event)
  {
    case WorldEvent::EditEnabled:
    case WorldEvent::EditDisabled:
      updateEnabled();
      break;

    case WorldEvent::PowerOff:
      if(m_kernel)
      {
        m_kernel->powerOff();
        m_kernel->emergencyStop();
      }
      break;

    case WorldEvent::PowerOn:
      if(m_kernel)
      {
        m_kernel->powerOn();
      }
      break;

    case WorldEvent::Stop:
      if(m_kernel)
      {
        m_kernel->emergencyStop();
      }
      updateEnabled();
      break;

    case WorldEvent::Run:
      if(m_kernel)
      {
        m_kernel->powerOn();
        m_kernel->clearEmergencyStop();
      }
      restoreDecoderSpeed();
      updateEnabled();
      break;

    default:
      break;
  }
}

void DCCEXInterface::check() const
{
  for(const auto& decoder : *decoders)
    checkDecoder(*decoder);
}

void DCCEXInterface::checkDecoder(const Decoder& decoder)
{
  for(const auto& function : *decoder.functions)
    if(function->number > DCCEX::Config::functionNumberMax)
    {
      Log::log(decoder, LogMessage::W2002_COMMAND_STATION_DOESNT_SUPPORT_FUNCTIONS_ABOVE_FX, DCCEX::Config::functionNumberMax);
      break;
    }
}

void DCCEXInterface::updateEnabled()
{
  const bool editable = contains(m_world.state, WorldState::Edit);
  const bool stopped = !contains(m_world.state, WorldState::Run);

  Attributes::setEnabled(dccex->speedSteps, editable && stopped);
}

void DCCEXInterface::updateVisible()
{
  const bool isSerial = (type == DCCEXInterfaceType::Serial);
  Attributes::setVisible(device, isSerial);
  Attributes::setVisible(baudrate, isSerial);

  const bool isNetwork = (type == DCCEXInterfaceType::NetworkTCP);
  Attributes::setVisible(hostname, isNetwork);
  Attributes::setVisible(port, isNetwork);
}
