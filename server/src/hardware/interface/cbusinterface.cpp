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
#include "../output/list/outputlist.hpp"
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
#include "../../world/world.hpp"

constexpr auto outputListColumns = OutputListColumn::Channel | OutputListColumn::Address;

CREATE_IMPL(CBUSInterface)

CBUSInterface::CBUSInterface(World& world, std::string_view _id)
  : Interface(world, _id)
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

std::span<const OutputChannel> CBUSInterface::outputChannels() const
{
  static const std::array<OutputChannel, 2> channels{
    OutputChannel::CBUSAccessoryShort,
    OutputChannel::CBUSAccessory,
    //OutputChannel::AccessoryDCC,
    //OutputChannel::DCCext,
  };
  return channels;
}

std::pair<uint32_t, uint32_t> CBUSInterface::outputAddressMinMax(OutputChannel channel) const
{
  switch(channel)
  {
    case OutputChannel::CBUSAccessory:
    case OutputChannel::CBUSAccessoryShort:
      return {std::numeric_limits<uint16_t>::min(), std::numeric_limits<uint16_t>::max()};

    default:
      return OutputController::outputAddressMinMax(channel);
  }
}

bool CBUSInterface::setOutputValue(OutputChannel channel, uint32_t address, OutputValue value)
{
  if(m_kernel)
  {
    switch(channel)
    {
      case OutputChannel::CBUSAccessoryShort:
        if(auto v = std::get<TriState>(value); v != TriState::Undefined)
        {
          m_kernel->setAccessoryShort(static_cast<uint16_t>(address), v == TriState::True);
          return true;
        }
        break;

      case OutputChannel::CBUSAccessory:
        if(auto v = std::get<TriState>(value); v != TriState::Undefined)
        {
          m_kernel->setAccessory(static_cast<uint16_t>(address), v == TriState::True);
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
          m_world.powerOff();
        };
      m_kernel->onTrackOn =
        [this]()
        {
          m_world.powerOn();
        };
      m_kernel->onEmergencyStop =
        [this]()
        {
          m_world.stop();
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
