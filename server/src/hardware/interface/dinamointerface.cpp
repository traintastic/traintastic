/**
 * server/src/hardware/interface/dinamointerface.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#include "dinamointerface.hpp"
#include "../protocol/dinamo/iohandler/serialiohandler.hpp"
#include "../protocol/dinamo/iohandler/simulationiohandler.hpp"
#include "../protocol/dinamo/kernel.hpp"
#include "../protocol/dinamo/settings.hpp"
#include "../../core/attributes.hpp"
#include "../../core/eventloop.hpp"
#include "../../core/objectproperty.tpp"
#include "../../log/log.hpp"
#include "../../log/logmessageexception.hpp"
#include "../../world/world.hpp"

CREATE_IMPL(DinamoInterface)

DinamoInterface::DinamoInterface(World& world, std::string_view objectId)
  : Interface(world, objectId)
  , device{this, "device", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , dinamo{this, "dinamo", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
{
  name = "DINAMO";
  dinamo.setValueInternal(std::make_shared<Dinamo::Settings>(*this, dinamo.name()));

  Attributes::addEnabled(device, false);
  m_interfaceItems.insertBefore(device, notes);

  m_interfaceItems.insertBefore(dinamo, notes);
}

DinamoInterface::~DinamoInterface() = default;

void DinamoInterface::addToWorld()
{
  Interface::addToWorld();
}

void DinamoInterface::destroying()
{
  Interface::destroying();
}

void DinamoInterface::worldEvent(WorldState state, WorldEvent event)
{
  Interface::worldEvent(state, event);

  if(m_kernel)
  {
    switch(event)
    {
      case WorldEvent::Stop:
        m_kernel->setFault();
        break;

      case WorldEvent::Run:
        m_kernel->resetFault();
        break;

      default:
        break;
    }
  }
}

bool DinamoInterface::setOnline(bool& value, bool simulation)
{
  if(!m_kernel && value)
  {
    try
    {
      if(simulation)
      {
        m_kernel = Dinamo::Kernel::create<Dinamo::SimulationIOHandler>(id.value(), dinamo->config());
      }
      else
      {
        m_kernel = Dinamo::Kernel::create<Dinamo::SerialIOHandler>(id.value(), dinamo->config(), device.value());
      }

      setState(InterfaceState::Initializing);

      m_kernel->setOnStarted(
        [this]()
        {
          assert(isEventLoopThread());
          setState(InterfaceState::Online);

          if(contains(m_world.state.value(), WorldState::Run))
          {
            m_kernel->resetFault();
          }
        });
      m_kernel->setOnError(
        [this]()
        {
          assert(isEventLoopThread());
          setState(InterfaceState::Error);
          online = false; // communication no longer possible
        });
      m_kernel->setOnFault(
        [this]()
        {
          assert(isEventLoopThread());
          if(contains(m_world.state.value(), WorldState::Run))
          {
            m_world.stop();
          }
        });
      m_kernel->start();

      m_dinamoPropertyChanged = dinamo->propertyChanged.connect(
        [this](BaseProperty& /*property*/)
        {
          m_kernel->setConfig(dinamo->config());
        });

      Attributes::setEnabled(device, false);
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
    Attributes::setEnabled(device, true);

    m_dinamoPropertyChanged.disconnect();

    m_kernel->stop();
    EventLoop::deleteLater(m_kernel.release());

    setState(InterfaceState::Offline);
  }
  return true;
}
