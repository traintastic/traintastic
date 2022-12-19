/**
 * server/src/hardware/interface/loconetinterface.cpp
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

#include "loconetinterface.hpp"
#include "../decoder/list/decoderlisttablemodel.hpp"
#include "../input/input.hpp"
#include "../identification/identification.hpp"
#include "../programming/lncv/lncvprogrammer.hpp"
#include "../protocol/loconet/iohandler/serialiohandler.hpp"
#include "../protocol/loconet/iohandler/simulationiohandler.hpp"
#include "../protocol/loconet/iohandler/tcpbinaryiohandler.hpp"
#include "../protocol/loconet/iohandler/lbserveriohandler.hpp"
#include "../protocol/loconet/iohandler/z21iohandler.hpp"
#include "../../core/attributes.hpp"
#include "../../log/log.hpp"
#include "../../log/logmessageexception.hpp"
#include "../../utils/displayname.hpp"
#include "../../utils/inrange.hpp"
#include "../../world/world.hpp"

constexpr auto decoderListColumns = DecoderListColumn::Id | DecoderListColumn::Name | DecoderListColumn::Address;
constexpr auto inputListColumns = InputListColumn::Id | InputListColumn::Name | InputListColumn::Address;
constexpr auto outputListColumns = OutputListColumn::Id | OutputListColumn::Name | OutputListColumn::Address;
constexpr auto identificationListColumns = IdentificationListColumn::Id | IdentificationListColumn::Name | IdentificationListColumn::Interface | IdentificationListColumn::Address;

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
  , hostname{this, "hostname", "192.168.1.203", PropertyFlags::ReadWrite | PropertyFlags::Store}
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

void LocoNetInterface::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(m_kernel)
    m_kernel->decoderChanged(decoder, changes, functionNumber);
}

void LocoNetInterface::inputSimulateChange(uint32_t channel, uint32_t address)
{
  if(m_kernel && inRange(address, outputAddressMinMax(channel)))
    m_kernel->simulateInputChange(address);
}

bool LocoNetInterface::setOutputValue(uint32_t channel, uint32_t address, bool value)
{
  return
    m_kernel &&
    inRange(address, outputAddressMinMax(channel)) &&
    m_kernel->setOutput(static_cast<uint16_t>(address), value);
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
        m_kernel = LocoNet::Kernel::create<LocoNet::SimulationIOHandler>(loconet->config());
      }
      else
      {
        switch(type)
        {
          case LocoNetInterfaceType::Serial:
            m_kernel = LocoNet::Kernel::create<LocoNet::SerialIOHandler>(loconet->config(), device.value(), baudrate.value(), flowControl.value());
            break;

          case LocoNetInterfaceType::TCPBinary:
            m_kernel = LocoNet::Kernel::create<LocoNet::TCPBinaryIOHandler>(loconet->config(), hostname.value(), port.value());
            break;

          case LocoNetInterfaceType::LBServer:
            m_kernel = LocoNet::Kernel::create<LocoNet::LBServerIOHandler>(loconet->config(), hostname.value(), port.value());
            break;

          case LocoNetInterfaceType::Z21:
            m_kernel = LocoNet::Kernel::create<LocoNet::Z21IOHandler>(loconet->config(), hostname.value());
            break;

          default:
            assert(false);
            return false;
        }
      }

      status.setValueInternal(InterfaceStatus::Initializing);

      m_kernel->setLogId(id.value());
      m_kernel->setOnStarted(
        [this]()
        {
          status.setValueInternal(InterfaceStatus::Online);
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

      m_kernel->setPowerOn(contains(m_world.state.value(), WorldState::PowerOn));

      if(contains(m_world.state.value(), WorldState::Run))
        m_kernel->resume();
      else
        m_kernel->emergencyStop();

      Attributes::setEnabled(type, false);
      Attributes::setEnabled(device, false);
      Attributes::setEnabled(baudrate, false);
      Attributes::setEnabled(flowControl, false);
      Attributes::setEnabled(hostname, false);
      Attributes::setEnabled(port, false);
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
    Attributes::setEnabled(type, true);
    Attributes::setEnabled(device, true);
    Attributes::setEnabled(baudrate, true);
    Attributes::setEnabled(flowControl, true);
    Attributes::setEnabled(hostname, true);
    Attributes::setEnabled(port, true);

    m_loconetPropertyChanged.disconnect();

    m_kernel->stop();
    m_kernel.reset();

    status.setValueInternal(InterfaceStatus::Offline);
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

void LocoNetInterface::idChanged(const std::string& newId)
{
  if(m_kernel)
    m_kernel->setLogId(newId);
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
