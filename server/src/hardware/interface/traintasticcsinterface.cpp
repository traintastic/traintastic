/**
 * server/src/hardware/interface/traintasticcsinterface.cpp
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

#include "traintasticcsinterface.hpp"
#include "../protocol/traintasticcs/kernel.hpp"
#include "../protocol/traintasticcs/settings.hpp"
#include "../protocol/traintasticcs/iohandler/serialiohandler.hpp"
#include "../protocol/traintasticcs/iohandler/simulationiohandler.hpp"
#include "../../core/attributes.hpp"
#include "../../core/eventloop.hpp"
#include "../../core/objectproperty.tpp"
#include "../../log/log.hpp"
#include "../../log/logmessageexception.hpp"

CREATE_IMPL(TraintasticCSInterface)

TraintasticCSInterface::TraintasticCSInterface(World& world, std::string_view _id)
  : Interface(world, _id)
  , device{this, "device", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , traintasticCS{this, "traintastic_cs", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
{
  name = "Traintastic CS";
  traintasticCS.setValueInternal(std::make_shared<TraintasticCS::Settings>(*this, traintasticCS.name()));

  Attributes::addEnabled(device, !online);
  m_interfaceItems.insertBefore(device, notes);

  m_interfaceItems.insertBefore(traintasticCS, notes);
}

bool TraintasticCSInterface::setOnline(bool& value, bool simulation)
{
  if(!m_kernel && value)
  {
    try
    {
      if(simulation)
      {
        m_kernel = TraintasticCS::Kernel::create<TraintasticCS::SimulationIOHandler>(id.value(), traintasticCS->config());
      }
      else
      {
        m_kernel = TraintasticCS::Kernel::create<TraintasticCS::SerialIOHandler>(id.value(), traintasticCS->config(), device.value());
      }

      if(!m_kernel)
      {
        assert(false);
        return false;
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
      m_kernel->start();

      m_traintasticCSPropertyChanged = traintasticCS->propertyChanged.connect(
        [this](BaseProperty& /*property*/)
        {
          m_kernel->setConfig(traintasticCS->config());
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

    m_traintasticCSPropertyChanged.disconnect();

    m_kernel->stop();
    EventLoop::deleteLater(m_kernel.release());

    setState(InterfaceState::Offline);
  }
  return true;
}

void TraintasticCSInterface::addToWorld()
{
  Interface::addToWorld();
}

void TraintasticCSInterface::destroying()
{
  Interface::destroying();
}
