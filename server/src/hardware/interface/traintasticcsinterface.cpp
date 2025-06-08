/**
 * server/src/hardware/interface/traintasticcsinterface.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024-2025 Reinder Feenstra
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
#include "../input/list/inputlist.hpp"
#include "../input/list/inputlistcolumn.hpp"
#include "../../throttle/list/throttlelistcolumn.hpp"
#include "../../core/attributes.hpp"
#include "../../core/eventloop.hpp"
#include "../../core/objectproperty.tpp"
#include "../../log/log.hpp"
#include "../../log/logmessageexception.hpp"
#include "../../utils/inrange.hpp"
#include "../../world/world.hpp"

static constexpr auto inputListColumns = InputListColumn::Id | InputListColumn::Name | InputListColumn::Channel | InputListColumn::Address;
static constexpr auto throttleListColumns = ThrottleListColumn::Name;

CREATE_IMPL(TraintasticCSInterface)

TraintasticCSInterface::TraintasticCSInterface(World& world, std::string_view _id)
  : Interface(world, _id)
  , InputController(static_cast<IdObject&>(*this))
  , ThrottleController(*this, throttleListColumns)
  , device{this, "device", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , traintasticCS{this, "traintastic_cs", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
{
  name = "Traintastic CS";
  traintasticCS.setValueInternal(std::make_shared<TraintasticCS::Settings>(*this, traintasticCS.name()));

  Attributes::addEnabled(device, false);
  m_interfaceItems.insertBefore(device, notes);

  m_interfaceItems.insertBefore(traintasticCS, notes);

  m_interfaceItems.insertBefore(inputs, notes);

  m_interfaceItems.insertBefore(throttles, notes);

  updateEnabled();
}

const std::vector<uint32_t>* TraintasticCSInterface::inputChannels() const
{
  return &TraintasticCS::Kernel::inputChannels;
}

const std::vector<std::string_view>* TraintasticCSInterface::inputChannelNames() const
{
  return &TraintasticCS::Kernel::inputChannelNames;
}

std::pair<uint32_t, uint32_t> TraintasticCSInterface::inputAddressMinMax(uint32_t channel) const
{
  using namespace TraintasticCS;

  switch(channel)
  {
    case Kernel::InputChannel::loconet:
      return {Kernel::loconetInputAddressMin, Kernel::loconetInputAddressMax};

    case Kernel::InputChannel::s88:
      return {Kernel::s88AddressMin, Kernel::s88AddressMax};
  }

  assert(false);
  return {0, 0};
}

void TraintasticCSInterface::inputSimulateChange(uint32_t channel, uint32_t address, SimulateInputAction action)
{
  if(m_kernel && inRange(address, inputAddressMinMax(channel)))
  {
    m_kernel->inputSimulateChange(channel, address, action);
  }
}

void TraintasticCSInterface::onlineChanged(bool /*value*/)
{
  updateEnabled();
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

      m_kernel->setInputController(this);
      m_kernel->setThrottleController(this);

      m_kernel->start();

      m_traintasticCSPropertyChanged = traintasticCS->propertyChanged.connect(
        [this](BaseProperty& /*property*/)
        {
          m_kernel->setConfig(traintasticCS->config());
        });
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
  InputController::addToWorld(inputListColumns);
}

void TraintasticCSInterface::destroying()
{
  InputController::destroying();
  Interface::destroying();
}

void TraintasticCSInterface::worldEvent(WorldState state, WorldEvent event)
{
  Interface::worldEvent(state, event);
  updateEnabled();
}

void TraintasticCSInterface::updateEnabled()
{
  const bool editable = contains(m_world.state, WorldState::Edit);

  Attributes::setEnabled(device, editable && !online);
  traintasticCS->updateEnabled(editable, online);
}
