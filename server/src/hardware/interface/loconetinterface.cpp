/**
 * server/src/hardware/interface/loconetinterface.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2024 Reinder Feenstra
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

#include "loconetinterface.hpp"
#include "../decoder/list/decoderlist.hpp"
#include "../decoder/list/decoderlisttablemodel.hpp"
#include "../input/input.hpp"
#include "../input/list/inputlist.hpp"
#include "../output/list/outputlist.hpp"
#include "../identification/list/identificationlist.hpp"
#include "../identification/identification.hpp"
#include "../programming/lncv/lncvprogrammer.hpp"
#include "../protocol/dcc/dcc.hpp"
#include "../protocol/loconet/kernel.hpp"
#include "../protocol/loconet/settings.hpp"
#include "../protocol/loconet/iohandler/serialiohandler.hpp"
#include "../protocol/loconet/iohandler/simulationiohandler.hpp"
#include "../protocol/loconet/iohandler/tcpbinaryiohandler.hpp"
#include "../protocol/loconet/iohandler/lbserveriohandler.hpp"
#include "../protocol/loconet/iohandler/z21iohandler.hpp"
#include "../../core/attributes.hpp"
#include "../../core/eventloop.hpp"
#include "../../core/method.tpp"
#include "../../core/objectproperty.tpp"
#include "../../log/log.hpp"
#include "../../log/logmessageexception.hpp"
#include "../../utils/displayname.hpp"
#include "../../utils/inrange.hpp"
#include "../../utils/makearray.hpp"
#include "../../world/world.hpp"

constexpr auto decoderListColumns = DecoderListColumn::Id | DecoderListColumn::Name | DecoderListColumn::Address;
constexpr auto inputListColumns = InputListColumn::Id | InputListColumn::Name | InputListColumn::Address;
constexpr auto outputListColumns = OutputListColumn::Channel | OutputListColumn::Address;
constexpr auto identificationListColumns = IdentificationListColumn::Id | IdentificationListColumn::Name | IdentificationListColumn::Interface | IdentificationListColumn::Address;

CREATE_IMPL(LocoNetInterface)

LocoNetInterface::LocoNetInterface(World& world, std::string_view _id)
  : Interface(world, _id)
  , DecoderController(*this, decoderListColumns)
  , InputController(static_cast<IdObject&>(*this))
  , OutputController(static_cast<IdObject&>(*this))
  , IdentificationController(static_cast<IdObject&>(*this))
  , type{this, "type", LocoNetInterfaceType::Serial, PropertyFlags::ReadWrite | PropertyFlags::Store,
      [this](LocoNetInterfaceType /*value*/)
      {
        typeChanged();
      }}
  , device{this, "device", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , baudrate{this, "baudrate", 19200, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , flowControl{this, "flow_control", SerialFlowControl::None, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , hostname{this, "hostname", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , port{this, "port", 5550, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , loconet{this, "loconet", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
{
  name = "LocoNet";
  loconet.setValueInternal(std::make_shared<LocoNet::Settings>(*this, loconet.name()));

  Attributes::addDisplayName(type, DisplayName::Interface::type);
  Attributes::addEnabled(type, !online);
  Attributes::addValues(type, locoNetInterfaceTypeValues);
  m_interfaceItems.insertBefore(type, notes);

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

  Attributes::addDisplayName(loconet, DisplayName::Hardware::loconet);
  m_interfaceItems.insertBefore(loconet, notes);

  m_interfaceItems.insertBefore(decoders, notes);

  m_interfaceItems.insertBefore(inputs, notes);

  m_interfaceItems.insertBefore(outputs, notes);

  m_interfaceItems.insertBefore(identifications, notes);

  typeChanged();
}

bool LocoNetInterface::send(tcb::span<uint8_t> packet)
{
  if(m_kernel)
    return m_kernel->send(packet);
  return false;
}

bool LocoNetInterface::immPacket(tcb::span<uint8_t> dccPacket, uint8_t repeat)
{
  if(m_kernel)
    return m_kernel->immPacket(dccPacket, repeat);
  return false;
}

tcb::span<const DecoderProtocol> LocoNetInterface::decoderProtocols() const
{
  static constexpr std::array<DecoderProtocol, 2> protocols{DecoderProtocol::DCCShort, DecoderProtocol::DCCLong};
  return tcb::span<const DecoderProtocol>{protocols.data(), protocols.size()};
}

std::pair<uint16_t, uint16_t> LocoNetInterface::decoderAddressMinMax(DecoderProtocol protocol) const
{
  if(protocol == DecoderProtocol::DCCLong)
  {
    return {DCC::addressLongStart, DCC::addressLongMax};
  }
  return DecoderController::decoderAddressMinMax(protocol);
}

void LocoNetInterface::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(m_kernel)
    m_kernel->decoderChanged(decoder, changes, functionNumber);
}

std::pair<uint32_t, uint32_t> LocoNetInterface::inputAddressMinMax(uint32_t) const
{
  return {LocoNet::Kernel::inputAddressMin, LocoNet::Kernel::inputAddressMax};
}

void LocoNetInterface::inputSimulateChange(uint32_t channel, uint32_t address, SimulateInputAction action)
{
  if(m_kernel && inRange(address, inputAddressMinMax(channel)))
    m_kernel->simulateInputChange(address, action);
}

tcb::span<const OutputChannel> LocoNetInterface::outputChannels() const
{
  static const auto values = makeArray(OutputChannel::Accessory, OutputChannel::DCCext);
  return values;
}

std::pair<uint32_t, uint32_t> LocoNetInterface::outputAddressMinMax(OutputChannel channel) const
{
  if(channel == OutputChannel::Accessory)
  {
    return {LocoNet::Kernel::accessoryOutputAddressMin, LocoNet::Kernel::accessoryOutputAddressMax};
  }
  return OutputController::outputAddressMinMax(channel);
}

bool LocoNetInterface::setOutputValue(OutputChannel channel, uint32_t address, OutputValue value)
{
  return
      m_kernel &&
      inRange(address, outputAddressMinMax(channel)) &&
      m_kernel->setOutput(channel, static_cast<uint16_t>(address), value);
}

std::pair<uint32_t, uint32_t> LocoNetInterface::identificationAddressMinMax(uint32_t) const
{
  return {LocoNet::Kernel::identificationAddressMin, LocoNet::Kernel::identificationAddressMax};
}

void LocoNetInterface::identificationEvent(uint32_t channel, uint32_t address, IdentificationEventType eventType, uint16_t identifier, Direction direction, uint8_t category)
{
  // OPC_MULTI_SENSE direction:
  if(direction == Direction::Unknown && (eventType == IdentificationEventType::Present || eventType == IdentificationEventType::Absent))
  {
    constexpr uint32_t addressDirectionMask = 0x800;

    if(auto it = m_identifications.find({channel, address}); it != m_identifications.end() )
    {
      switch(it->second->opcMultiSenseDirection.value())
      {
        case OPCMultiSenseDirection::None:
          break;

        case OPCMultiSenseDirection::InSensorAddress:
          direction = Direction::Reverse;
          break;

        case OPCMultiSenseDirection::InTransponderAddress:
        {
          constexpr uint16_t identifierDirectionMask = 0x1000;
          direction = (identifier & identifierDirectionMask) ? Direction::Forward : Direction::Reverse;
          identifier &= ~identifierDirectionMask;
          break;
        }
      }
    }
    else if(address & addressDirectionMask)
    {
      address &= ~addressDirectionMask;

      if(it = m_identifications.find({channel, address});
          it != m_identifications.end() &&
          it->second->opcMultiSenseDirection == OPCMultiSenseDirection::InSensorAddress)
      {
        direction = Direction::Forward;
      }
    }
  }

  IdentificationController::identificationEvent(channel, address, eventType, identifier, direction, category);
}

bool LocoNetInterface::startLNCVProgramming(uint16_t moduleId, uint16_t moduleAddress)
{
  if(!m_kernel)
    return false;

  m_kernel->lncvStart(moduleId, moduleAddress);
  return true;
}

bool LocoNetInterface::readLNCV(uint16_t lncv)
{
  if(!m_kernel)
    return false;

  m_kernel->lncvRead(lncv);
  return true;
}

bool LocoNetInterface::writeLNCV(uint16_t lncv, uint16_t value)
{
  if(!m_kernel)
    return false;

  m_kernel->lncvWrite(lncv, value);
  return true;
}

bool LocoNetInterface::stopLNCVProgramming()
{
  if(!m_kernel)
    return false;

  m_kernel->lncvStop();
  return true;
}

bool LocoNetInterface::setOnline(bool& value, bool simulation)
{
  if(!m_kernel && value)
  {
    try
    {
      if(simulation)
      {
        m_kernel = LocoNet::Kernel::create<LocoNet::SimulationIOHandler>(id.value(), loconet->config());
      }
      else
      {
        switch(type)
        {
          case LocoNetInterfaceType::Serial:
            m_kernel = LocoNet::Kernel::create<LocoNet::SerialIOHandler>(id.value(), loconet->config(), device.value(), baudrate.value(), flowControl.value());
            break;

          case LocoNetInterfaceType::TCPBinary:
            m_kernel = LocoNet::Kernel::create<LocoNet::TCPBinaryIOHandler>(id.value(), loconet->config(), hostname.value(), port.value());
            break;

          case LocoNetInterfaceType::LBServer:
            m_kernel = LocoNet::Kernel::create<LocoNet::LBServerIOHandler>(id.value(), loconet->config(), hostname.value(), port.value());
            break;

          case LocoNetInterfaceType::Z21:
            m_kernel = LocoNet::Kernel::create<LocoNet::Z21IOHandler>(id.value(), loconet->config(), hostname.value());
            break;

          default:
            assert(false);
            return false;
        }
      }

      setState(InterfaceState::Initializing);

      m_kernel->setOnStarted(
        [this]()
        {
          setState(InterfaceState::Online);

          m_kernel->setPowerOn(contains(m_world.state.value(), WorldState::PowerOn));

          if(contains(m_world.state.value(), WorldState::Run))
            m_kernel->resume();
          else
            m_kernel->emergencyStop();
        });
      m_kernel->setOnError(
        [this]()
        {
          setState(InterfaceState::Error);
          online = false; // communication no longer possible
        });
      m_kernel->setOnGlobalPowerChanged(
        [this](bool powerOn)
        {
          if(powerOn && !contains(m_world.state.value(), WorldState::PowerOn))
            m_world.powerOn();
          else if(!powerOn && contains(m_world.state.value(), WorldState::PowerOn))
            m_world.powerOff();
        });
      m_kernel->setOnIdle(
        [this]()
        {
          if(contains(m_world.state.value(), WorldState::Run))
            m_world.stop();
        });
      m_kernel->setClock(m_world.clock.value());
      m_kernel->setDecoderController(this);
      m_kernel->setInputController(this);
      m_kernel->setOutputController(this);
      m_kernel->setIdentificationController(this);

      m_kernel->setOnLNCVReadResponse(
        [this](bool success, uint16_t lncv, uint16_t lncvValue)
        {
          if(auto* programmer = lncvProgrammer())
            programmer->readResponse(success, lncv, lncvValue);
        });

      m_kernel->start();

      m_loconetPropertyChanged = loconet->propertyChanged.connect(
        [this](BaseProperty& /*property*/)
        {
          m_kernel->setConfig(loconet->config());
        });

      Attributes::setEnabled(type, false);
      Attributes::setEnabled(device, false);
      Attributes::setEnabled(baudrate, false);
      Attributes::setEnabled(flowControl, false);
      Attributes::setEnabled(hostname, false);
      Attributes::setEnabled(port, false);
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
    Attributes::setEnabled(type, true);
    Attributes::setEnabled(device, true);
    Attributes::setEnabled(baudrate, true);
    Attributes::setEnabled(flowControl, true);
    Attributes::setEnabled(hostname, true);
    Attributes::setEnabled(port, true);

    m_loconetPropertyChanged.disconnect();

    m_kernel->stop();
    EventLoop::deleteLater(m_kernel.release());

    if(status->state != InterfaceState::Error)
      setState(InterfaceState::Offline);
  }
  return true;
}

void LocoNetInterface::addToWorld()
{
  Interface::addToWorld();
  DecoderController::addToWorld();
  InputController::addToWorld(inputListColumns);
  OutputController::addToWorld(outputListColumns);
  IdentificationController::addToWorld(identificationListColumns);
  LNCVProgrammingController::addToWorld();
}

void LocoNetInterface::loaded()
{
  Interface::loaded();

  typeChanged();
}

void LocoNetInterface::destroying()
{
  LNCVProgrammingController::destroying();
  IdentificationController::destroying();
  OutputController::destroying();
  InputController::destroying();
  DecoderController::destroying();
  Interface::destroying();
}

void LocoNetInterface::worldEvent(WorldState state, WorldEvent event)
{
  Interface::worldEvent(state, event);

  if(m_kernel)
  {
    switch(event)
    {
      case WorldEvent::PowerOff:
        m_kernel->setPowerOn(false);
        break;

      case WorldEvent::PowerOn:
        m_kernel->setPowerOn(true);
        if(contains(state, WorldState::Run))
          m_kernel->resume();
        break;

      case WorldEvent::Stop:
        m_kernel->emergencyStop();
        break;

      case WorldEvent::Run:
        if(contains(state, WorldState::PowerOn))
          m_kernel->resume();
        break;

      default:
        break;
    }
  }
}

void LocoNetInterface::typeChanged()
{
  const bool serialVisible = isSerial(type);
  Attributes::setVisible(device, serialVisible);
  Attributes::setVisible(baudrate, serialVisible);
  Attributes::setVisible(flowControl, serialVisible);

  const bool networkVisible = isNetwork(type);
  Attributes::setVisible(hostname, networkVisible);
  Attributes::setVisible(port, networkVisible && type != LocoNetInterfaceType::Z21);
}
