/**
 * server/src/hardware/interface/wlanmausinterface.cpp
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

#include "wlanmausinterface.hpp"
#include "../protocol/z21/iohandler/udpserveriohandler.hpp"
#include "../../core/attributes.hpp"
#include "../../log/log.hpp"
#include "../../log/logmessageexception.hpp"
#include "../../utils/displayname.hpp"
#include "../../world/world.hpp"

WlanMausInterface::WlanMausInterface(const std::weak_ptr<World>& world, std::string_view _id)
  : Interface(world, _id)
  , z21{this, "z21", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
{
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

void WlanMausInterface::idChanged(const std::string& newId)
{
  if(m_kernel)
    m_kernel->setLogId(newId);
}

bool WlanMausInterface::setOnline(bool& value)
{
  if(!m_kernel && value)
  {
    try
    {
      auto world = m_world.lock();
      assert(world);
      m_kernel = Z21::ServerKernel::create<Z21::UDPServerIOHandler>(z21->config(), world->decoders.value());

      status.setValueInternal(InterfaceStatus::Initializing);

      m_kernel->setLogId(id.value());
      m_kernel->setOnStarted(
        [this]()
        {
          status.setValueInternal(InterfaceStatus::Online);
        });

      m_kernel->setOnTrackPowerOff(
        [this]()
        {
          if(auto w = m_world.lock(); w && contains(w->state.value(), WorldState::PowerOn))
            w->powerOff();
        });
      m_kernel->setOnTrackPowerOn(
        [this]()
        {
          if(auto w = m_world.lock())
          {
            if(!contains(w->state.value(), WorldState::PowerOn))
              w->powerOn();
            if(!contains(w->state.value(), WorldState::Run))
              w->run();
          }
        });
      m_kernel->setOnEmergencyStop(
        [this]()
        {
          if(auto w = m_world.lock(); w && contains(w->state.value(), WorldState::Run))
            w->stop();
        });

      m_kernel->start();

      m_z21PropertyChanged = z21->propertyChanged.connect(
        [this](BaseProperty& /*property*/)
        {
          m_kernel->setConfig(z21->config());
        });

      if(auto w = m_world.lock())
        m_kernel->setState(contains(w->state.value(), WorldState::PowerOn), !contains(w->state.value(), WorldState::Run));
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
    m_kernel->stop();
    m_kernel.reset();

    status.setValueInternal(InterfaceStatus::Offline);
  }
  return true;
}
