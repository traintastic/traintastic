/**
 * server/src/hardware/interface/xpressnetinterface.cpp
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

#include "xpressnetinterface.hpp"
#include "../decoder/list/decoderlist.hpp"
#include "../input/input.hpp"
#include "../input/list/inputlist.hpp"
#include "../output/list/outputlist.hpp"
#include "../protocol/xpressnet/kernel.hpp"
#include "../protocol/xpressnet/settings.hpp"
#include "../protocol/xpressnet/messages.hpp"
#include "../protocol/xpressnet/iohandler/serialiohandler.hpp"
#include "../protocol/xpressnet/iohandler/simulationiohandler.hpp"
#include "../protocol/xpressnet/iohandler/liusbiohandler.hpp"
#include "../protocol/xpressnet/iohandler/rosofts88xpressnetliiohandler.hpp"
#include "../protocol/xpressnet/iohandler/tcpiohandler.hpp"
#include "../../core/attributes.hpp"
#include "../../core/method.tpp"
#include "../../core/objectproperty.tpp"
#include "../../log/log.hpp"
#include "../../log/logmessageexception.hpp"
#include "../../utils/displayname.hpp"
#include "../../utils/inrange.hpp"
#include "../../world/world.hpp"

constexpr auto decoderListColumns = DecoderListColumn::Id | DecoderListColumn::Name | DecoderListColumn::Address;
constexpr auto inputListColumns = InputListColumn::Id | InputListColumn::Name | InputListColumn::Address;
constexpr auto outputListColumns = OutputListColumn::Id | OutputListColumn::Name | OutputListColumn::Address;

CREATE_IMPL(XpressNetInterface)

XpressNetInterface::XpressNetInterface(World& world, std::string_view _id)
  : Interface(world, _id)
  , DecoderController(*this, decoderListColumns)
  , InputController(static_cast<IdObject&>(*this))
  , OutputController(static_cast<IdObject&>(*this))
  , type{this, "type", XpressNetInterfaceType::Serial, PropertyFlags::ReadWrite | PropertyFlags::Store,
      [this](XpressNetInterfaceType /*value*/)
      {
        updateVisible();
      }}
  , serialInterfaceType{this, "interface", XpressNetSerialInterfaceType::LenzLI100, PropertyFlags::ReadWrite | PropertyFlags::Store,
      [this](XpressNetSerialInterfaceType value)
      {
        switch(value)
        {
          case XpressNetSerialInterfaceType::LenzLI100:
          case XpressNetSerialInterfaceType::RoSoftS88XPressNetLI:
            baudrate.setValueInternal(9600);
            flowControl.setValueInternal(SerialFlowControl::Hardware);
            break;

          case XpressNetSerialInterfaceType::LenzLI100F:
          case XpressNetSerialInterfaceType::LenzLI101F:
            baudrate.setValueInternal(19200);
            flowControl.setValueInternal(SerialFlowControl::Hardware);
            break;

          case XpressNetSerialInterfaceType::LenzLIUSB:
            baudrate.setValueInternal(57600);
            flowControl.setValueInternal(SerialFlowControl::None);
            break;

          case XpressNetSerialInterfaceType::DigikeijsDR5000:
            baudrate.setValueInternal(115200);
            flowControl.setValueInternal(SerialFlowControl::None);
            break;
        }
        updateVisible();
      }}
  , device{this, "device", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , baudrate{this, "baudrate", 19200, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , flowControl{this, "flow_control", SerialFlowControl::None, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , hostname{this, "hostname", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , port{this, "port", 5550, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , s88StartAddress{this, "s88_start_address", XpressNet::RoSoftS88XpressNetLI::S88StartAddress::startAddressDefault, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , s88ModuleCount{this, "s88_module_count", XpressNet::RoSoftS88XpressNetLI::S88ModuleCount::moduleCountDefault, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , xpressnet{this, "xpressnet", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
{
  name = "XpressNet";
  xpressnet.setValueInternal(std::make_shared<XpressNet::Settings>(*this, xpressnet.name()));

  Attributes::addDisplayName(type, DisplayName::Interface::type);
  Attributes::addEnabled(type, !online);
  Attributes::addValues(type, xpressNetInterfaceTypeValues);
  m_interfaceItems.insertBefore(type, notes);

  Attributes::addValues(serialInterfaceType, XpressNetSerialInterfaceTypeValues);
  Attributes::addEnabled(serialInterfaceType, !online);
  Attributes::addVisible(serialInterfaceType, false);
  m_interfaceItems.insertBefore(serialInterfaceType, notes);

  Attributes::addEnabled(device, !online);
  Attributes::addVisible(device, false);
  m_interfaceItems.insertBefore(device, notes);

  Attributes::addDisplayName(baudrate, DisplayName::Serial::baudrate);
  Attributes::addEnabled(baudrate, !online);
  Attributes::addVisible(baudrate, false);
  m_interfaceItems.insertBefore(baudrate, notes);

  Attributes::addDisplayName(flowControl, DisplayName::Serial::flowControl);
  Attributes::addEnabled(flowControl, !online);
  Attributes::addValues(flowControl, SerialFlowControlValues);
  Attributes::addVisible(flowControl, false);
  m_interfaceItems.insertBefore(flowControl, notes);

  Attributes::addDisplayName(hostname, DisplayName::IP::hostname);
  Attributes::addEnabled(hostname, !online);
  Attributes::addVisible(hostname, false);
  m_interfaceItems.insertBefore(hostname, notes);

  Attributes::addDisplayName(port, DisplayName::IP::port);
  Attributes::addEnabled(port, !online);
  Attributes::addVisible(port, false);
  m_interfaceItems.insertBefore(port, notes);

  Attributes::addMinMax(s88StartAddress, XpressNet::RoSoftS88XpressNetLI::S88StartAddress::startAddressMin, XpressNet::RoSoftS88XpressNetLI::S88StartAddress::startAddressMax);
  Attributes::addEnabled(s88StartAddress, !online);
  Attributes::addVisible(s88StartAddress, false);
  m_interfaceItems.insertBefore(s88StartAddress, notes);

  Attributes::addMinMax(s88ModuleCount, XpressNet::RoSoftS88XpressNetLI::S88ModuleCount::moduleCountMin, XpressNet::RoSoftS88XpressNetLI::S88ModuleCount::moduleCountMax);
  Attributes::addEnabled(s88ModuleCount, !online);
  Attributes::addVisible(s88ModuleCount, false);
  m_interfaceItems.insertBefore(s88ModuleCount, notes);

  Attributes::addDisplayName(xpressnet, DisplayName::Hardware::xpressnet);
  m_interfaceItems.insertBefore(xpressnet, notes);

  m_interfaceItems.insertBefore(decoders, notes);

  m_interfaceItems.insertBefore(inputs, notes);

  m_interfaceItems.insertBefore(outputs, notes);

  updateVisible();
}

tcb::span<const DecoderProtocol> XpressNetInterface::decoderProtocols() const
{
  static constexpr std::array<DecoderProtocol, 2> protocols{DecoderProtocol::DCCShort, DecoderProtocol::DCCLong};
  return tcb::span<const DecoderProtocol>{protocols.data(), protocols.size()};
}

std::pair<uint16_t, uint16_t> XpressNetInterface::decoderAddressMinMax(DecoderProtocol protocol) const
{
  switch(protocol)
  {
    case DecoderProtocol::DCCShort:
      return {XpressNet::shortAddressMin, XpressNet::shortAddressMax};

    case DecoderProtocol::DCCLong:
      return {XpressNet::longAddressMin, XpressNet::longAddressMax};

    default: /*[[unlikely]]*/
      return DecoderController::decoderAddressMinMax(protocol);
  }
}

tcb::span<const uint8_t> XpressNetInterface::decoderSpeedSteps(DecoderProtocol protocol) const
{
  static constexpr std::array<uint8_t, 4> dccSpeedSteps{{14, 27, 28, 128}}; // XpressNet also support 27 steps

  switch(protocol)
  {
    case DecoderProtocol::DCCShort:
    case DecoderProtocol::DCCLong:
      return dccSpeedSteps;

    default: /*[[unlikely]]*/
      return DecoderController::decoderSpeedSteps(protocol);
  }
}

void XpressNetInterface::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(m_kernel)
    m_kernel->decoderChanged(decoder, changes, functionNumber);
}

std::pair<uint32_t, uint32_t> XpressNetInterface::inputAddressMinMax(uint32_t) const
{
  return {XpressNet::Kernel::ioAddressMin, XpressNet::Kernel::ioAddressMax};
}

void XpressNetInterface::inputSimulateChange(uint32_t channel, uint32_t address, SimulateInputAction action)
{
  if(m_kernel && inRange(address, outputAddressMinMax(channel)))
    m_kernel->simulateInputChange(address, action);
}

std::pair<uint32_t, uint32_t> XpressNetInterface::outputAddressMinMax(uint32_t) const
{
  return {XpressNet::Kernel::ioAddressMin, XpressNet::Kernel::ioAddressMax};
}

bool XpressNetInterface::setOutputValue(uint32_t channel, uint32_t address, bool value)
{
  assert(isOutputChannel(channel));
  return
      m_kernel &&
      inRange(address, outputAddressMinMax(channel)) &&
    m_kernel->setOutput(static_cast<uint16_t>(address), value);
}

bool XpressNetInterface::setOnline(bool& value, bool simulation)
{
  if(!m_kernel && value)
  {
    try
    {
      if(simulation)
      {
        m_kernel = XpressNet::Kernel::create<XpressNet::SimulationIOHandler>(xpressnet->config());
      }
      else
      {
        switch(type)
        {
          case XpressNetInterfaceType::Serial:
            switch(serialInterfaceType)
            {
              case XpressNetSerialInterfaceType::LenzLI100:
              case XpressNetSerialInterfaceType::LenzLI100F:
              case XpressNetSerialInterfaceType::LenzLI101F:
                m_kernel = XpressNet::Kernel::create<XpressNet::SerialIOHandler>(xpressnet->config(), device.value(), baudrate.value(), flowControl.value());
                break;

              case XpressNetSerialInterfaceType::RoSoftS88XPressNetLI:
                m_kernel = XpressNet::Kernel::create<XpressNet::RoSoftS88XPressNetLIIOHandler>(xpressnet->config(), device.value(), baudrate.value(), flowControl.value(), s88StartAddress.value(), s88ModuleCount.value());
                break;

              case XpressNetSerialInterfaceType::LenzLIUSB:
              case XpressNetSerialInterfaceType::DigikeijsDR5000:
                m_kernel = XpressNet::Kernel::create<XpressNet::LIUSBIOHandler>(xpressnet->config(), device.value(), baudrate.value(), flowControl.value());
                break;
            }
            break;

          case XpressNetInterfaceType::Network:
            m_kernel = XpressNet::Kernel::create<XpressNet::TCPIOHandler>(xpressnet->config(), hostname.value(), port.value());
            break;
        }
      }

      if(!m_kernel)
      {
        assert(false);
        return false;
      }

      setState(InterfaceState::Initializing);

      m_kernel->setLogId(id.value());
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
      m_kernel->setOnNormalOperationResumed(
        [this]()
        {
          if(!contains(m_world.state.value(), WorldState::PowerOn))
            m_world.powerOn();
          if(!contains(m_world.state.value(), WorldState::Run))
            m_world.run();
        });
      m_kernel->setOnTrackPowerOff(
        [this]()
        {
          if(contains(m_world.state.value(), WorldState::PowerOn))
            m_world.powerOff();
          if(contains(m_world.state.value(), WorldState::Run))
            m_world.stop();
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

      m_xpressnetPropertyChanged = xpressnet->propertyChanged.connect(
        [this](BaseProperty& /*property*/)
        {
          m_kernel->setConfig(xpressnet->config());
        });

      if(!contains(m_world.state.value(), WorldState::PowerOn))
        m_kernel->stopOperations();
      else if(!contains(m_world.state.value(), WorldState::Run))
        m_kernel->stopAllLocomotives();
      else
        m_kernel->resumeOperations();

      Attributes::setEnabled({type, serialInterfaceType, device, baudrate, flowControl, hostname, port, s88StartAddress, s88ModuleCount}, false);
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
    Attributes::setEnabled({type, serialInterfaceType, device, baudrate, flowControl, hostname, port, s88StartAddress, s88ModuleCount}, true);

    m_xpressnetPropertyChanged.disconnect();

    m_kernel->stop();
    m_kernel.reset();

    setState(InterfaceState::Offline);
  }
  return true;
}

void XpressNetInterface::addToWorld()
{
  Interface::addToWorld();
  DecoderController::addToWorld();
  InputController::addToWorld(inputListColumns);
  OutputController::addToWorld(outputListColumns);
}

void XpressNetInterface::loaded()
{
  Interface::loaded();

  updateVisible();
}

void XpressNetInterface::destroying()
{
  OutputController::destroying();
  InputController::destroying();
  DecoderController::destroying();
  Interface::destroying();
}

void XpressNetInterface::worldEvent(WorldState state, WorldEvent event)
{
  Interface::worldEvent(state, event);

  if(m_kernel)
  {
    switch(event)
    {
      case WorldEvent::PowerOff:
        m_kernel->stopOperations();
        break;

      case WorldEvent::PowerOn:
        m_kernel->resumeOperations();
        if(!contains(state, WorldState::Run))
          m_kernel->stopAllLocomotives();
        break;

      case WorldEvent::Stop:
        m_kernel->stopAllLocomotives();
        break;

      case WorldEvent::Run:
        if(contains(state, WorldState::PowerOn))
          m_kernel->resumeOperations();
        break;

      default:
        break;
    }
  }
}

void XpressNetInterface::idChanged(const std::string& newId)
{
  if(m_kernel)
    m_kernel->setLogId(newId);
}

void XpressNetInterface::updateVisible()
{
  const bool isSerial = (type == XpressNetInterfaceType::Serial);
  Attributes::setVisible(serialInterfaceType, isSerial);
  Attributes::setVisible(device, isSerial);
  Attributes::setVisible(baudrate, isSerial);
  Attributes::setVisible(flowControl, isSerial);

  const bool isNetwork = (type == XpressNetInterfaceType::Network);
  Attributes::setVisible(hostname, isNetwork);
  Attributes::setVisible(port, isNetwork);

  const bool isRoSoftS88XPressNetLI = isSerial && (serialInterfaceType == XpressNetSerialInterfaceType::RoSoftS88XPressNetLI);
  Attributes::setVisible(s88StartAddress, isRoSoftS88XPressNetLI);
  Attributes::setVisible(s88ModuleCount, isRoSoftS88XPressNetLI);
}
