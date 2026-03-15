/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#include "cbusinterface.hpp"
#include "cbus/cbussettings.hpp"
#include "../decoder/decoderchangeflags.hpp"
#include "../decoder/list/decoderlist.hpp"
#include "../decoder/list/decoderlisttablemodel.hpp"
#include "../input/list/inputlist.hpp"
#include "../output/list/outputlist.hpp"
#include "../protocol/cbus/cbusconst.hpp"
#include "../protocol/cbus/cbuskernel.hpp"
#include "../protocol/cbus/iohandler/cbuscanusbiohandler.hpp"
#include "../protocol/cbus/iohandler/cbuscanetheriohandler.hpp"
#include "../protocol/cbus/iohandler/cbussimulationiohandler.hpp"
#include "../protocol/cbus/simulator/cbussimulator.hpp"
#include "../protocol/cbus/simulator/module/cbuscancmd.hpp"
#include "../../core/attributes.hpp"
#include "../../core/eventloop.hpp"
#include "../../core/method.tpp"
#include "../../core/objectproperty.tpp"
#include "../../log/log.hpp"
#include "../../log/logmessageexception.hpp"
#include "../../utils/displayname.hpp"
#include "../../utils/inrange.hpp"
#include "../../world/world.hpp"

constexpr auto decoderListColumns = DecoderListColumn::Id | DecoderListColumn::Name | DecoderListColumn::Address;
constexpr auto inputListColumns = InputListColumn::Channel | InputListColumn::Node | InputListColumn::Address;
constexpr auto outputListColumns = OutputListColumn::Channel | OutputListColumn::Node | OutputListColumn::Address;

constexpr auto nodeNumberRange = std::make_pair(std::numeric_limits<uint16_t>::min(), std::numeric_limits<uint16_t>::max());
constexpr auto eventNumberRange = std::make_pair(std::numeric_limits<uint16_t>::min(), std::numeric_limits<uint16_t>::max());

CREATE_IMPL(CBUSInterface)

CBUSInterface::CBUSInterface(World& world, std::string_view _id)
  : Interface(world, _id)
  , DecoderController(*this, decoderListColumns)
  , InputController(static_cast<IdObject&>(*this))
  , OutputController(static_cast<IdObject&>(*this))
  , type{this, "type", CBUSInterfaceType::CANUSB, PropertyFlags::ReadWrite | PropertyFlags::Store,
      [this](CBUSInterfaceType /*value*/)
      {
        updateVisible();
      }}
  , device{this, "device", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , hostname{this, "hostname", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , port{this, "port", 0, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , cbus{this, "cbus", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
{
  name = "CBUS/VLCB";
  cbus.setValueInternal(std::make_shared<CBUSSettings>(*this, cbus.name()));

  Attributes::addDisplayName(type, DisplayName::Interface::type);
  Attributes::addEnabled(type, !online);
  Attributes::addValues(type, CBUSInterfaceTypeValues);
  m_interfaceItems.insertBefore(type, notes);

  Attributes::addEnabled(device, !online);
  Attributes::addVisible(device, false);
  m_interfaceItems.insertBefore(device, notes);

  Attributes::addDisplayName(hostname, DisplayName::IP::hostname);
  Attributes::addEnabled(hostname, !online);
  Attributes::addVisible(hostname, false);
  m_interfaceItems.insertBefore(hostname, notes);

  Attributes::addDisplayName(port, DisplayName::IP::port);
  Attributes::addEnabled(port, !online);
  Attributes::addVisible(port, false);
  m_interfaceItems.insertBefore(port, notes);

  m_interfaceItems.insertBefore(cbus, notes);

  m_interfaceItems.insertBefore(decoders, notes);

  m_interfaceItems.insertBefore(inputs, notes);

  m_interfaceItems.insertBefore(outputs, notes);

  m_cbusPropertyChanged = cbus->propertyChanged.connect(
    [this](BaseProperty& /*property*/)
    {
      if(m_kernel)
      {
        m_kernel->setConfig(cbus->config());
      }
    });

  updateVisible();
}

CBUSInterface::~CBUSInterface() = default;

bool CBUSInterface::send(std::vector<uint8_t> message)
{
  if(m_kernel)
  {
    return m_kernel->send(std::move(message));
  }
  return false;
}

bool CBUSInterface::sendDCC(std::vector<uint8_t> dccPacket, uint8_t repeat)
{
  if(m_kernel)
  {
    return m_kernel->sendDCC(std::move(dccPacket), repeat);
  }
  return false;
}

std::span<const DecoderProtocol> CBUSInterface::decoderProtocols() const
{
  static constexpr std::array<DecoderProtocol, 2> protocols{DecoderProtocol::DCCShort, DecoderProtocol::DCCLong};
  return std::span<const DecoderProtocol>{protocols.data(), protocols.size()};
}

void CBUSInterface::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(m_kernel)
  {
    const bool longAddress = (decoder.protocol == DecoderProtocol::DCCLong);

    if(has(changes, DecoderChangeFlags::FunctionValue) && functionNumber <= CBUS::engineFunctionMax)
    {
      m_kernel->setEngineFunction(
        decoder.address,
        longAddress,
        static_cast<uint8_t>(functionNumber),
        decoder.getFunctionValue(functionNumber));
    }
    else
    {
      m_kernel->setEngineSpeedDirection(
        decoder.address,
        longAddress,
        Decoder::throttleToSpeedStep<uint8_t>(decoder.throttle, decoder.speedSteps),
        decoder.speedSteps,
        decoder.emergencyStop,
        decoder.direction == Direction::Forward);
    }
  }
}

std::span<const InputChannel> CBUSInterface::inputChannels() const
{
  static const std::array<InputChannel, 2> channels{
    InputChannel::ShortEvent,
    InputChannel::LongEvent,
  };
  return channels;
}

bool CBUSInterface::isInputLocation(InputChannel channel, const InputLocation& location) const
{
  if(hasNodeAddressLocation(channel))
  {
    return
      inRange<uint32_t>(std::get<InputNodeAddress>(location).node, nodeNumberRange) &&
      inRange<uint32_t>(std::get<InputNodeAddress>(location).address, inputAddressMinMax(channel));
  }
  return InputController::isInputLocation(channel, location);
}

std::pair<uint32_t, uint32_t> CBUSInterface::inputAddressMinMax(InputChannel /*channel*/) const
{
  return eventNumberRange;
}

void CBUSInterface::inputSimulateChange(InputChannel channel, const InputLocation& location, SimulateInputAction action)
{
  if(m_simulator)
  {
    m_kernel->ioContext().post(
      [this, channel, location, action]
      {
        switch(channel)
        {
          case InputChannel::ShortEvent:
            m_simulator->shortEvent(std::get<InputAddress>(location).address, action);
            break;

          case InputChannel::LongEvent:
            m_simulator->longEvent(std::get<InputNodeAddress>(location).node, std::get<InputNodeAddress>(location).address, action);
            break;

          default: [[unlikely]]
            assert(false);
            break;
        }
      });
  }
}

std::span<const OutputChannel> CBUSInterface::outputChannels() const
{
  static const std::array<OutputChannel, 2> channels{
    OutputChannel::ShortEvent,
    OutputChannel::LongEvent,
    //OutputChannel::AccessoryDCC,
    //OutputChannel::DCCext,
  };
  return channels;
}

std::pair<uint32_t, uint32_t> CBUSInterface::outputNodeMinMax(OutputChannel /*channel*/) const
{
  return eventNumberRange;
}

std::pair<uint32_t, uint32_t> CBUSInterface::outputAddressMinMax(OutputChannel channel) const
{
  switch(channel)
  {
    case OutputChannel::LongEvent:
    case OutputChannel::ShortEvent:
      return {std::numeric_limits<uint16_t>::min(), std::numeric_limits<uint16_t>::max()};

    default:
      return OutputController::outputAddressMinMax(channel);
  }
}

bool CBUSInterface::setOutputValue(OutputChannel channel, const OutputLocation& location, OutputValue value)
{
  if(m_kernel)
  {
    switch(channel)
    {
      case OutputChannel::ShortEvent:
        if(auto v = std::get<TriState>(value); v != TriState::Undefined)
        {
          const auto address = static_cast<uint16_t>(std::get<OutputAddress>(location).address);
          m_kernel->setAccessoryShort(address, v == TriState::True);
          return true;
        }
        break;

      case OutputChannel::LongEvent:
        if(auto v = std::get<TriState>(value); v != TriState::Undefined)
        {
          const auto node = static_cast<uint16_t>(std::get<OutputNodeAddress>(location).node);
          const auto address = static_cast<uint16_t>(std::get<OutputNodeAddress>(location).address);
          m_kernel->setAccessory(node, address, v == TriState::True);
          return true;
        }
        break;

      default: [[unlikely]]
        assert(false);
        break;
    }
  }
  return false;
}

void CBUSInterface::addToWorld()
{
  Interface::addToWorld();
  DecoderController::addToWorld();
  InputController::addToWorld(inputListColumns);
  OutputController::addToWorld(outputListColumns);
}

void CBUSInterface::loaded()
{
  Interface::loaded();

  updateVisible();
}

void CBUSInterface::destroying()
{
  m_cbusPropertyChanged.disconnect();
  OutputController::destroying();
  InputController::destroying();
  DecoderController::destroying();
  Interface::destroying();
}

void CBUSInterface::worldEvent(WorldState state, WorldEvent event)
{
  Interface::worldEvent(state, event);

  switch(event)
  {
    case WorldEvent::PowerOff:
      if(m_kernel)
      {
        m_kernel->trackOff();
      }
      break;

    case WorldEvent::PowerOn:
      if(m_kernel)
      {
        m_kernel->trackOn();
      }
      break;

    case WorldEvent::Stop:
      if(m_kernel)
      {
        m_kernel->requestEmergencyStop();
      }
      break;

    case WorldEvent::Run:
      if(m_kernel)
      {
        // TODO: send all known speed values
      }
      break;

    default:
      break;
  }
}

bool CBUSInterface::setOnline(bool& value, bool simulation)
{
  if(!m_kernel && value)
  {
    try
    {
      if(simulation)
      {
        m_simulator = std::make_unique<CBUS::Simulator>();
        m_simulator->addModule(std::make_unique<CBUS::Module::CANCMD>(*m_simulator));
        m_kernel = CBUS::Kernel::create<CBUS::SimulationIOHandler>(id.value(), cbus->config(), std::ref(*m_simulator));
      }
      else
      {
        switch(type)
        {
          case CBUSInterfaceType::CANUSB:
            m_kernel = CBUS::Kernel::create<CBUS::CANUSBIOHandler>(id.value(), cbus->config(), device.value());
            break;

          case CBUSInterfaceType::CANEther:
            m_kernel = CBUS::Kernel::create<CBUS::CANEtherIOHandler>(id.value(), cbus->config(), hostname.value(), port.value());
            break;
        }
      }

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
      m_kernel->onTrackOff =
        [this]()
        {
          if(contains(m_world.state, WorldState::PowerOn))
          {
            m_world.powerOff();
          }
        };
      m_kernel->onTrackOn =
        [this]()
        {
          if(!contains(m_world.state, WorldState::PowerOn))
          {
            m_world.powerOn();
          }
        };
      m_kernel->onEmergencyStop =
        [this]()
        {
          if(contains(m_world.state, WorldState::Run))
          {
            m_world.stop();
          }
        };
      m_kernel->onShortEvent =
        [this](uint16_t eventNumber, bool on)
        {
          updateInputValue(InputChannel::ShortEvent, InputAddress(eventNumber), toTriState(on));
          updateOutputValue(OutputChannel::ShortEvent, OutputAddress(eventNumber), toTriState(on));
        };
      m_kernel->onLongEvent =
        [this](uint16_t nodeNumber, uint16_t eventNumber, bool on)
        {
          updateInputValue(InputChannel::LongEvent, InputNodeAddress(nodeNumber, eventNumber), toTriState(on));
          updateOutputValue(OutputChannel::LongEvent, OutputNodeAddress(nodeNumber, eventNumber), toTriState(on));
        };

      m_kernel->start();
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
    m_kernel->stop();
    EventLoop::deleteLater(m_kernel.release());
    EventLoop::deleteLater(m_simulator.release());

    if(status->state != InterfaceState::Error)
    {
      setState(InterfaceState::Offline);
    }
  }
  return true;
}

void CBUSInterface::updateVisible()
{
  const bool isSerial = (type == CBUSInterfaceType::CANUSB);
  Attributes::setVisible(device, isSerial);

  const bool isNetwork = (type == CBUSInterfaceType::CANEther);
  Attributes::setVisible(hostname, isNetwork);
  Attributes::setVisible(port, isNetwork);
}
