/**
 * server/src/hardware/interface/dccplusplusinterface.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2023 Reinder Feenstra
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

#include "dccplusplusinterface.hpp"
#include "../decoder/list/decoderlist.hpp"
#include "../decoder/list/decoderlisttablemodel.hpp"
#include "../input/list/inputlist.hpp"
#include "../output/list/outputlist.hpp"
#include "../protocol/dcc/dcc.hpp"
#include "../protocol/dccplusplus/kernel.hpp"
#include "../protocol/dccplusplus/settings.hpp"
#include "../protocol/dccplusplus/messages.hpp"
#include "../protocol/dccplusplus/iohandler/serialiohandler.hpp"
#include "../protocol/dccplusplus/iohandler/simulationiohandler.hpp"
#include "../../core/attributes.hpp"
#include "../../core/method.tpp"
#include "../../core/objectproperty.tpp"
#include "../../log/log.hpp"
#include "../../log/logmessageexception.hpp"
#include "../../utils/displayname.hpp"
#include "../../utils/inrange.hpp"
#include "../../utils/serialport.hpp"
#include "../../world/world.hpp"

constexpr auto decoderListColumns = DecoderListColumn::Id | DecoderListColumn::Name | DecoderListColumn::Address;
constexpr auto inputListColumns = InputListColumn::Id | InputListColumn::Name | InputListColumn::Address;
constexpr auto outputListColumns = OutputListColumn::Id | OutputListColumn::Name | OutputListColumn::Channel | OutputListColumn::Address;

CREATE_IMPL(DCCPlusPlusInterface)

DCCPlusPlusInterface::DCCPlusPlusInterface(World& world, std::string_view _id)
  : Interface(world, _id)
  , DecoderController(*this, decoderListColumns)
  , InputController(static_cast<IdObject&>(*this))
  , OutputController(static_cast<IdObject&>(*this))
  , device{this, "device", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , baudrate{this, "baudrate", 115200, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , dccplusplus{this, "dccplusplus", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
{
  name = "DCC++";
  dccplusplus.setValueInternal(std::make_shared<DCCPlusPlus::Settings>(*this, dccplusplus.name()));

  Attributes::addEnabled(device, !online);
  m_interfaceItems.insertBefore(device, notes);

  Attributes::addDisplayName(baudrate, DisplayName::Serial::baudrate);
  Attributes::addEnabled(baudrate, !online);
  Attributes::addMinMax(baudrate, SerialPort::baudrateMin, SerialPort::baudrateMax);
  Attributes::addValues(baudrate, SerialPort::baudrateValues);
  m_interfaceItems.insertBefore(baudrate, notes);

  Attributes::addDisplayName(dccplusplus, DisplayName::Hardware::dccplusplus);
  m_interfaceItems.insertBefore(dccplusplus, notes);

  m_interfaceItems.insertBefore(decoders, notes);

  m_interfaceItems.insertBefore(inputs, notes);

  m_interfaceItems.insertBefore(outputs, notes);

  m_dccplusplusPropertyChanged = dccplusplus->propertyChanged.connect(
    [this](BaseProperty& property)
    {
      if(m_kernel && &property != &dccplusplus->startupDelay)
        m_kernel->setConfig(dccplusplus->config());

      if(&property == &dccplusplus->speedSteps)
      {
        // update speedsteps of all decoders, DCC++ only has a global speedsteps setting
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
}

tcb::span<const DecoderProtocol> DCCPlusPlusInterface::decoderProtocols() const
{
  static constexpr std::array<DecoderProtocol, 2> protocols{DecoderProtocol::DCCShort, DecoderProtocol::DCCLong};
  return tcb::span<const DecoderProtocol>{protocols.data(), protocols.size()};
}

std::pair<uint16_t, uint16_t> DCCPlusPlusInterface::decoderAddressMinMax(DecoderProtocol protocol) const
{
  if(protocol == DecoderProtocol::DCCLong)
    return {DCC::addressLongStart, DCC::addressLongMax}; // DCC++ considers all addresses below 128 as short.
  return DecoderController::decoderAddressMinMax(protocol);
}

tcb::span<const uint8_t> DCCPlusPlusInterface::decoderSpeedSteps(DecoderProtocol protocol) const
{
  (void)protocol; // silence unused warning for release build
  assert(protocol == DecoderProtocol::DCCShort || protocol == DecoderProtocol::DCCLong);
  const auto& speedStepValues = DCCPlusPlus::Settings::speedStepValues;
  // find value in array so we can create a span, using a span of a variable won't work due to the compare with prevous value in the attribute setter
  if(auto it = std::find(speedStepValues.begin(), speedStepValues.end(), dccplusplus->speedSteps); it != speedStepValues.end()) /*[[likely]]/*/
    return {&(*it), 1};
  assert(false);
  return {};
}

void DCCPlusPlusInterface::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(m_kernel)
    m_kernel->decoderChanged(decoder, changes, functionNumber);
}

std::pair<uint32_t, uint32_t> DCCPlusPlusInterface::inputAddressMinMax(uint32_t) const
{
  return {DCCPlusPlus::Kernel::idMin, DCCPlusPlus::Kernel::idMax};
}

void DCCPlusPlusInterface::inputSimulateChange(uint32_t channel, uint32_t address, SimulateInputAction action)
{
  if(m_kernel && inRange(address, inputAddressMinMax(channel)))
    m_kernel->simulateInputChange(address, action);
}

const std::vector<uint32_t> *DCCPlusPlusInterface::outputChannels() const
{
  return &DCCPlusPlus::Kernel::outputChannels;
}

const std::vector<std::string_view> *DCCPlusPlusInterface::outputChannelNames() const
{
  return &DCCPlusPlus::Kernel::outputChannelNames;
}

std::pair<uint32_t, uint32_t> DCCPlusPlusInterface::outputAddressMinMax(uint32_t channel) const
{
  using namespace DCCPlusPlus;

  switch(channel)
  {
    case Kernel::OutputChannel::dccAccessory:
      return {Kernel::dccAccessoryAddressMin, Kernel::dccAccessoryAddressMax};

    case Kernel::OutputChannel::turnout:
    case Kernel::OutputChannel::output:
      return {Kernel::idMin, Kernel::idMax};
  }

  assert(false);
  return {0, 0};
}

bool DCCPlusPlusInterface::setOutputValue(uint32_t channel, uint32_t address, bool value)
{
  return
    m_kernel &&
    inRange(address, outputAddressMinMax(channel)) &&
    m_kernel->setOutput(channel, static_cast<uint16_t>(address), value);
}

bool DCCPlusPlusInterface::setOnline(bool& value, bool simulation)
{
  if(!m_kernel && value)
  {
    try
    {
      if(simulation)
      {
        m_kernel = DCCPlusPlus::Kernel::create<DCCPlusPlus::SimulationIOHandler>(id.value(), dccplusplus->config());
      }
      else
      {
        m_kernel = DCCPlusPlus::Kernel::create<DCCPlusPlus::SerialIOHandler>(id.value(), dccplusplus->config(), device.value(), baudrate.value(), SerialFlowControl::None);
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

      Attributes::setEnabled({device, baudrate}, false);
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
    Attributes::setEnabled({device, baudrate}, true);

    m_kernel->stop();
    m_kernel.reset();

    setState(InterfaceState::Offline);
  }
  return true;
}

void DCCPlusPlusInterface::addToWorld()
{
  Interface::addToWorld();
  DecoderController::addToWorld();
  InputController::addToWorld(inputListColumns);
  OutputController::addToWorld(outputListColumns);
}

void DCCPlusPlusInterface::loaded()
{
  Interface::loaded();

  check();
  updateEnabled();
}

void DCCPlusPlusInterface::destroying()
{
  m_dccplusplusPropertyChanged.disconnect();
  OutputController::destroying();
  InputController::destroying();
  DecoderController::destroying();
  Interface::destroying();
}

void DCCPlusPlusInterface::worldEvent(WorldState state, WorldEvent event)
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

void DCCPlusPlusInterface::check() const
{
  for(const auto& decoder : *decoders)
    checkDecoder(*decoder);
}

void DCCPlusPlusInterface::checkDecoder(const Decoder& decoder) const
{
  for(const auto& function : *decoder.functions)
    if(function->number > DCCPlusPlus::Config::functionNumberMax)
    {
      Log::log(decoder, LogMessage::W2002_COMMAND_STATION_DOESNT_SUPPORT_FUNCTIONS_ABOVE_FX, DCCPlusPlus::Config::functionNumberMax);
      break;
    }
}

void DCCPlusPlusInterface::updateEnabled()
{
  const bool editable = contains(m_world.state, WorldState::Edit);
  const bool stopped = !contains(m_world.state, WorldState::Run);

  Attributes::setEnabled(dccplusplus->speedSteps, editable && stopped);
}
