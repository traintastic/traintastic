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

#include "rclinkinterface.hpp"
#include "rclinksettings.hpp"
#include "../../input/list/inputlist.hpp"
#include "../../identification/list/identificationlist.hpp"
#include "../../protocol/rclink/rclinkconst.hpp"
#include "../../protocol/rclink/rclinkkernel.hpp"
#include "../../protocol/rclink/rclinkmessages.hpp"
#include "../../protocol/rclink/iohandler/rclinkserialiohandler.hpp"
#include "../../protocol/rclink/iohandler/rclinksimulationiohandler.hpp"
#include "../../protocol/rclink/simulator/rclinksimulator.hpp"
#include "../../../core/attributes.hpp"
#include "../../../core/eventloop.hpp"
#include "../../../core/objectproperty.tpp"
#include "../../../log/log.hpp"
#include "../../../log/logmessageexception.hpp"
#include "../../../world/world.hpp"

constexpr auto inputListColumns = InputListColumn::Address;
constexpr auto identificationListColumns = IdentificationListColumn::Id | IdentificationListColumn::Name | IdentificationListColumn::Address;

CREATE_IMPL(RCLinkInterface)

RCLinkInterface::RCLinkInterface(World& world, std::string_view _id)
  : Interface(world, _id)
  , InputController(static_cast<IdObject&>(*this))
  , IdentificationController(static_cast<IdObject&>(*this))
  , device{this, "device", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , rcLink{this, "rc_link", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
{
  name = "RC-Link";
  rcLink.setValueInternal(std::make_shared<RCLinkSettings>(*this, rcLink.name()));

  Attributes::addEnabled(device, !online);
  m_interfaceItems.insertBefore(device, notes);

  m_interfaceItems.insertBefore(rcLink, notes);

  m_interfaceItems.insertBefore(inputs, notes);

  m_interfaceItems.insertBefore(identifications, notes);

  m_rcLinkPropertyChanged = rcLink->propertyChanged.connect(
    [this](BaseProperty& /*property*/)
    {
      if(m_kernel)
      {
        m_kernel->setConfig(rcLink->config());
      }
    });

  updateEnabled();
}

RCLinkInterface::~RCLinkInterface() = default;

std::span<const InputChannel> RCLinkInterface::inputChannels() const
{
  static constexpr std::array<InputChannel, 1> values{{InputChannel::Input}};
  return values;
}

std::pair<uint32_t, uint32_t> RCLinkInterface::inputAddressMinMax(InputChannel /*channel*/) const
{
  return {RCLink::inputAddressMin, RCLink::inputAddressMax};
}

void RCLinkInterface::inputSimulateChange(InputChannel /*channel*/, const InputLocation& location, SimulateInputAction action)
{
  if(m_simulator) [[likely]]
  {
    assert(std::holds_alternative<InputAddress>(location));
    const auto address = static_cast<uint8_t>(std::get<InputAddress>(location).address);
    switch(action)
    {
      using enum SimulateInputAction;

      case SetFalse:
        m_simulator->detectorEvent(address, false);
        break;

      case SetTrue:
        m_simulator->detectorEvent(address, true);
        break;

      case Toggle:
        m_simulator->detectorEventToggle(address);
        break;
    }
  }
}

std::pair<uint32_t, uint32_t> RCLinkInterface::identificationAddressMinMax(uint32_t /*channel*/) const
{
  return {RCLink::inputAddressMin, RCLink::inputAddressMax};
}

bool RCLinkInterface::setOnline(bool& value, bool simulation)
{
  if(!m_kernel && value)
  {
    try
    {
      if(simulation)
      {
        m_simulator = std::make_unique<RCLink::Simulator>();
        m_kernel = RCLink::Kernel::create<RCLink::SimulationIOHandler>(id.value(), rcLink->config(), std::ref(*m_simulator));
      }
      else
      {
        m_kernel = RCLink::Kernel::create<RCLink::SerialIOHandler>(id.value(), rcLink->config(), device.value());
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
      m_kernel->onDetector =
        [this](uint8_t address, bool occupied, uint16_t railcomAddress, bool orientation)
        {
          updateInputValue(InputChannel::Input, InputAddress(address), toTriState(occupied));

          if(railcomAddress != 0)
          {
            identificationEvent(
              IdentificationController::defaultIdentificationChannel,
              address,
              occupied ? IdentificationEventType::Present : IdentificationEventType::Absent,
              railcomAddress,
              orientation ? Direction::Forward : Direction::Reverse,
              0);
          }
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

void RCLinkInterface::onlineChanged(bool /*value*/)
{
  updateEnabled();
}

void RCLinkInterface::addToWorld()
{
  Interface::addToWorld();
  InputController::addToWorld(inputListColumns);
  IdentificationController::addToWorld(identificationListColumns);
}

void RCLinkInterface::loaded()
{
  Interface::loaded();

  updateEnabled();
}

void RCLinkInterface::destroying()
{
  m_rcLinkPropertyChanged.disconnect();
  IdentificationController::destroying();
  InputController::destroying();
  Interface::destroying();
}

void RCLinkInterface::worldEvent(WorldState state, WorldEvent event)
{
  Interface::worldEvent(state, event);

  switch(event)
  {
    case WorldEvent::EditEnabled:
    case WorldEvent::EditDisabled:
      updateEnabled();
      break;

    default:
      break;
  }
}

void RCLinkInterface::updateEnabled()
{
  const bool editable = contains(m_world.state, WorldState::Edit);

  Attributes::setEnabled(device, !online && editable);
}
