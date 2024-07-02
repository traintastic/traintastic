/**
 * server/src/hardware/interface/wlanmausinterface.cpp
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

#include "wlanmausinterface.hpp"
#include "../protocol/z21/serverkernel.hpp"
#include "../protocol/z21/serversettings.hpp"
#include "../protocol/z21/iohandler/udpserveriohandler.hpp"
#include "../../core/attributes.hpp"
#include "../../core/eventloop.hpp"
#include "../../core/method.tpp"
#include "../../core/objectproperty.tpp"
#include "../../log/log.hpp"
#include "../../log/logmessageexception.hpp"
#include "../../utils/displayname.hpp"
#include "../../world/world.hpp"

CREATE_IMPL(WlanMausInterface)

WlanMausInterface::WlanMausInterface(World& world, std::string_view _id)
  : Interface(world, _id)
  , z21{this, "z21", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
{
  name = "WLANmaus";
  z21.setValueInternal(std::make_shared<Z21::ServerSettings>(*this, z21.name()));

  Attributes::addDisplayName(z21, DisplayName::Hardware::z21);
  m_interfaceItems.insertBefore(z21, notes);
}

void WlanMausInterface::worldEvent(WorldState state, WorldEvent event)
{
  Interface::worldEvent(state, event);

  if(m_kernel)
  {
    switch(event)
    {
      case WorldEvent::PowerOff:
      case WorldEvent::PowerOn:
      case WorldEvent::Stop:
      case WorldEvent::Run:
        m_kernel->setState(contains(state, WorldState::PowerOn), !contains(state, WorldState::Run));
        break;

      default:
        break;
    }
  }
}

bool WlanMausInterface::setOnline(bool& value, bool simulation)
{
  if(simulation)
  {
    Log::log(*this, LogMessage::N2001_SIMULATION_NOT_SUPPORTED);
    return false;
  }

  if(!m_kernel && value)
  {
    try
    {
      m_kernel = Z21::ServerKernel::create<Z21::UDPServerIOHandler>(id.value(), z21->config(), m_world.decoders.value());

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
      m_kernel->setOnTrackPowerOff(
        [this]()
        {
          if(contains(m_world.state.value(), WorldState::PowerOn))
            m_world.powerOff();
        });
      m_kernel->setOnTrackPowerOn(
        [this]()
        {
          if(!contains(m_world.state.value(), WorldState::PowerOn))
            m_world.powerOn();
          if(!contains(m_world.state.value(), WorldState::Run))
            m_world.run();
        });
      m_kernel->setOnEmergencyStop(
        [this]()
        {
          if(contains(m_world.state.value(), WorldState::Run))
            m_world.stop();
        });

      m_kernel->start();

      m_z21PropertyChanged = z21->propertyChanged.connect(
        [this](BaseProperty& /*property*/)
        {
          m_kernel->setConfig(z21->config());
        });

      m_kernel->setState(contains(m_world.state.value(), WorldState::PowerOn), !contains(m_world.state.value(), WorldState::Run));
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
    m_z21PropertyChanged.disconnect();

    m_kernel->stop();
    EventLoop::deleteLater(m_kernel.release());

    setState(InterfaceState::Offline);
  }
  return true;
}
