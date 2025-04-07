/**
 * server/src/hardware/interface/marklincaninterface.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023-2025 Reinder Feenstra
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

#include "marklincaninterface.hpp"
#include "../decoder/list/decoderlist.hpp" // ????
#include "../decoder/list/decoderlisttablemodel.hpp"
#include "../input/input.hpp"
#include "../input/list/inputlist.hpp"
#include "../output/list/outputlist.hpp"
#include "../protocol/marklincan/iohandler/simulationiohandler.hpp"
#include "../protocol/marklincan/iohandler/tcpiohandler.hpp"
#include "../protocol/marklincan/iohandler/udpiohandler.hpp"
#ifdef __linux__
  #include "../protocol/marklincan/iohandler/socketcaniohandler.hpp"
#endif
#include "../protocol/marklincan/iohandler/serialiohandler.hpp"
#include "../protocol/marklincan/kernel.hpp"
#include "../protocol/marklincan/settings.hpp"
#include "../../core/attributes.hpp"
#include "../../core/eventloop.hpp"
#include "../../core/objectproperty.tpp"
#include "../../log/log.hpp"
#include "../../log/logmessageexception.hpp"
#include "../../utils/displayname.hpp"
#include "../../utils/inrange.hpp"
#include "../../utils/makearray.hpp"

constexpr auto decoderListColumns = DecoderListColumn::Id | DecoderListColumn::Name | DecoderListColumn::Protocol | DecoderListColumn::Address;
constexpr auto inputListColumns = InputListColumn::Id | InputListColumn::Name | InputListColumn::Address;
constexpr auto outputListColumns = OutputListColumn::Channel | OutputListColumn::Address;

MarklinCANInterface::MarklinCANInterface(World& world, std::string_view _id)
  : Interface(world, _id)
  , DecoderController(*this, decoderListColumns)
  , InputController(static_cast<IdObject&>(*this))
  , OutputController(static_cast<IdObject&>(*this))
  , type{this, "type", MarklinCANInterfaceType::NetworkTCP, PropertyFlags::ReadWrite | PropertyFlags::Store,
      [this](MarklinCANInterfaceType /*value*/)
      {
        typeChanged();
      }}
  , hostname{this, "hostname", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , interface{this, "interface", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , device{this, "device", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , baudrate{this, "baudrate", 115'200, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , flowControl{this, "flow_control", SerialFlowControl::None, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , marklinCAN{this, "marklin_can", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
  , marklinCANNodeList{this, "marklin_can_node_list", nullptr, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::SubObject}
  , marklinCANLocomotiveList{this, "marklin_can_locomotive_list", nullptr, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::SubObject}
{
  name = "M\u00E4rklin CAN";
  marklinCAN.setValueInternal(std::make_shared<MarklinCAN::Settings>(*this, marklinCAN.name()));
  marklinCANNodeList.setValueInternal(std::make_shared<MarklinCANNodeList>(*this, marklinCANNodeList.name()));
  marklinCANLocomotiveList.setValueInternal(std::make_shared<MarklinCANLocomotiveList>(*this, marklinCANLocomotiveList.name()));

  Attributes::addDisplayName(type, DisplayName::Interface::type);
  Attributes::addEnabled(type, !online);
  Attributes::addValues(type, marklinCANInterfaceTypeValues);
  m_interfaceItems.insertBefore(type, notes);

  Attributes::addDisplayName(hostname, DisplayName::IP::hostname);
  Attributes::addEnabled(hostname, !online);
  Attributes::addVisible(hostname, false);
  m_interfaceItems.insertBefore(hostname, notes);

  Attributes::addDisplayName(interface, DisplayName::Hardware::interface);
  Attributes::addEnabled(interface, !online);
  Attributes::addVisible(interface, false);
  m_interfaceItems.insertBefore(interface, notes);

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

  Attributes::addDisplayName(marklinCAN, DisplayName::Hardware::marklinCAN);
  m_interfaceItems.insertBefore(marklinCAN, notes);

  m_interfaceItems.insertBefore(marklinCANNodeList, notes);

  m_interfaceItems.insertBefore(marklinCANLocomotiveList, notes);

  m_interfaceItems.insertBefore(decoders, notes);

  m_interfaceItems.insertBefore(inputs, notes);

  m_interfaceItems.insertBefore(outputs, notes);

  typeChanged();
}

std::span<const DecoderProtocol> MarklinCANInterface::decoderProtocols() const
{
  static constexpr std::array<DecoderProtocol, 4> protocols{DecoderProtocol::DCCShort, DecoderProtocol::DCCLong, DecoderProtocol::MFX, DecoderProtocol::Motorola};
  return std::span<const DecoderProtocol>{protocols.data(), protocols.size()};
}

std::span<const uint8_t> MarklinCANInterface::decoderSpeedSteps(DecoderProtocol protocol) const
{
  static constexpr std::array<uint8_t, 2> dccLongSpeedSteps{{28, 128}}; // 14 not supported for long addresses

  switch(protocol)
  {
    case DecoderProtocol::DCCLong:
      return dccLongSpeedSteps;

    default:
      return DecoderController::decoderSpeedSteps(protocol);
  }
}

void MarklinCANInterface::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(m_kernel)
    m_kernel->decoderChanged(decoder, changes, functionNumber);
}

std::pair<uint32_t, uint32_t> MarklinCANInterface::inputAddressMinMax(uint32_t /*channel*/) const
{
  return {MarklinCAN::Kernel::s88AddressMin, MarklinCAN::Kernel::s88AddressMax};
}

std::span<const OutputChannel> MarklinCANInterface::outputChannels() const
{
  static const auto values = makeArray(OutputChannel::AccessoryMotorola, OutputChannel::AccessoryDCC);
  return values;
}

bool MarklinCANInterface::setOutputValue(OutputChannel channel, uint32_t address, OutputValue value)
{
  return
    m_kernel &&
    inRange(address, outputAddressMinMax(channel)) &&
    m_kernel->setOutput(channel, static_cast<uint16_t>(address), std::get<OutputPairValue>(value));
}

bool MarklinCANInterface::setOnline(bool& value, bool simulation)
{
  if(!m_kernel && value)
  {
    try
    {
      if(simulation)
      {
        m_kernel = MarklinCAN::Kernel::create<MarklinCAN::SimulationIOHandler>(id.value(), marklinCAN->config());
      }
      else
      {
        switch(type.value())
        {
          case MarklinCANInterfaceType::NetworkTCP:
            m_kernel = MarklinCAN::Kernel::create<MarklinCAN::TCPIOHandler>(id.value(), marklinCAN->config(), hostname.value());
            break;

          case MarklinCANInterfaceType::NetworkUDP:
            m_kernel = MarklinCAN::Kernel::create<MarklinCAN::UDPIOHandler>(id.value(), marklinCAN->config(), hostname.value());
            break;

          case MarklinCANInterfaceType::SocketCAN:
#ifdef __linux__
            m_kernel = MarklinCAN::Kernel::create<MarklinCAN::SocketCANIOHandler>(id.value(), marklinCAN->config(), interface.value());
            break;
#else
            setState(InterfaceState::Error);
            Log::log(*this, LogMessage::C2005_SOCKETCAN_IS_ONLY_AVAILABLE_ON_LINUX);
            return false;
#endif
          case MarklinCANInterfaceType::Serial:
            m_kernel = MarklinCAN::Kernel::create<MarklinCAN::SerialIOHandler>(id.value(), marklinCAN->config(), device.value(), baudrate.value(), flowControl.value());
            break;
        }
      }
      assert(m_kernel);

      setState(InterfaceState::Initializing);

      m_kernel->setOnStarted(
        [this]()
        {
          setState(InterfaceState::Online);
          Attributes::setEnabled(marklinCANLocomotiveList->reload, true);
        });
      m_kernel->setOnError(
        [this]()
        {
          setState(InterfaceState::Error);
          online = false; // communication no longer possible
        });
      m_kernel->setOnNodeChanged(
        [this](const MarklinCAN::Node& node)
        {
          marklinCANNodeList->update(node);
        });
      m_kernel->setOnLocomotiveListChanged(
        [this](const std::shared_ptr<MarklinCAN::LocomotiveList>& list)
        {
          marklinCANLocomotiveList->setData(list);
        });

      m_kernel->setDecoderController(this);
      m_kernel->setInputController(this);
      m_kernel->setOutputController(this);

      m_kernel->start();

      m_marklinCANPropertyChanged = marklinCAN->propertyChanged.connect(
        [this](BaseProperty& /*property*/)
        {
          m_kernel->setConfig(marklinCAN->config());
        });

      Attributes::setEnabled({type, hostname, interface, device, baudrate, flowControl}, false);
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
    Attributes::setEnabled({type, hostname, interface, device, baudrate, flowControl}, true);
    Attributes::setEnabled(marklinCANLocomotiveList->reload, false);

    marklinCANNodeList->clear();
    marklinCANLocomotiveList->clear();

    m_marklinCANPropertyChanged.disconnect();

    m_kernel->stop();
    EventLoop::deleteLater(m_kernel.release());

    if(status->state != InterfaceState::Error)
      setState(InterfaceState::Offline);
  }
  return true;
}

void MarklinCANInterface::addToWorld()
{
  Interface::addToWorld();
  DecoderController::addToWorld();
  InputController::addToWorld(inputListColumns);
  OutputController::addToWorld(outputListColumns);
}

void MarklinCANInterface::loaded()
{
  Interface::loaded();

  typeChanged();
}

void MarklinCANInterface::destroying()
{
  OutputController::destroying();
  InputController::destroying();
  DecoderController::destroying();
  Interface::destroying();
}

void MarklinCANInterface::worldEvent(WorldState state, WorldEvent event)
{
  Interface::worldEvent(state, event);

  if(m_kernel)
  {
    switch(event)
    {
      case WorldEvent::PowerOff:
        m_kernel->systemStop();
        break;

      case WorldEvent::PowerOn:
        m_kernel->systemGo();
        m_kernel->systemHalt();
        break;

      case WorldEvent::Stop:
        m_kernel->systemHalt();
        break;

      case WorldEvent::Run:
        m_kernel->systemGo();
        break;

      default:
        break;
    }
  }
}

void MarklinCANInterface::typeChanged()
{
  Attributes::setVisible(hostname, isNetwork(type));
  Attributes::setVisible(interface, type == MarklinCANInterfaceType::SocketCAN);
  Attributes::setVisible({device, baudrate, flowControl}, type == MarklinCANInterfaceType::Serial);
}
