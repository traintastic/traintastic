/**
 * server/src/hardware/interface/withrottleinterface.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2023 Reinder Feenstra
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

#include "withrottleinterface.hpp"
#include "../throttle/list/throttlelistcolumn.hpp"
#include "../protocol/withrottle/kernel.hpp"
#include "../protocol/withrottle/settings.hpp"
#include "../protocol/withrottle/iohandler/tcpiohandler.hpp"
#include "../../core/attributes.hpp"
#include "../../core/method.tpp"
#include "../../core/objectproperty.tpp"
#include "../../log/log.hpp"
#include "../../log/logmessageexception.hpp"
#include "../../utils/displayname.hpp"
#include "../../world/world.hpp"

static constexpr auto throttleListColumns = ThrottleListColumn::Id | ThrottleListColumn::Name;

CREATE_IMPL(WiThrottleInterface)

WiThrottleInterface::WiThrottleInterface(World& world, std::string_view _id)
  : Interface(world, _id)
  , ThrottleController(*this, throttleListColumns)
  , port{this, "port", 4444, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , wiThrottle{this, "withrottle", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
{
  name = "WiThrottle";
  wiThrottle.setValueInternal(std::make_shared<WiThrottle::Settings>(*this, wiThrottle.name()));

  Attributes::addDisplayName(port, DisplayName::IP::port);
  Attributes::addEnabled(port, !online);
  m_interfaceItems.insertBefore(port, notes);

  m_interfaceItems.insertBefore(wiThrottle, notes);

  m_interfaceItems.insertBefore(throttles, notes);
}

void WiThrottleInterface::worldEvent(WorldState state, WorldEvent event)
{
  Interface::worldEvent(state, event);

  if(m_kernel)
  {
    switch(event)
    {
      case WorldEvent::PowerOff:
      case WorldEvent::PowerOn:
      case WorldEvent::Run:
        m_kernel->setPowerOn(contains(state, WorldState::PowerOn));
        break;

      default:
        break;
    }
  }
}

bool WiThrottleInterface::setOnline(bool& value, bool simulation)
{
  if(!m_kernel && value)
  {
    if(simulation)
    {
      Log::log(*this, LogMessage::N2001_SIMULATION_NOT_SUPPORTED);
    }

    try
    {
      m_kernel = WiThrottle::Kernel::create<WiThrottle::TCPIOHandler>(id.value(), wiThrottle->config(), port.value());

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

      m_kernel->setClock(world().clock.value());
      m_kernel->setThrottleController(this);

      m_kernel->start();

      m_kernel->setPowerOn(contains(m_world.state.value(), WorldState::PowerOn));

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
    Attributes::setEnabled(port, true);

    m_kernel->stop();
    m_kernel.reset();

    setState(InterfaceState::Offline);
  }

  return true;
}
