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

#include "cbusboosterdriver.hpp"
#include "../../interface/cbusinterface.hpp"
#include "../../protocol/cbus/cbusconst.hpp"
#include "../../protocol/cbus/messages/cbusaccessorymessages.hpp"
#include "../../../core/attributes.hpp"
#include "../../../core/objectproperty.tpp"
#include "../../../utils/displayname.hpp"
#include "../../../world/getworld.hpp"
#include "../../../world/world.hpp"

namespace {

constexpr float currentScale = 0.001f; //!< 1mA steps
constexpr float voltageScale = 0.1f; //!< 100mV steps

}

CBUSBoosterDriver::CBUSBoosterDriver(Booster& booster)
  : BoosterDriver(booster)
  , interface{this, "interface", nullptr, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript,
      [this](const std::shared_ptr<CBUSInterface>& value)
      {
        interfaceOnlineChanged(value->online);
      },
      [this](const std::shared_ptr<CBUSInterface>& value)
      {
        if(interface)
        {
          m_interfacePropertyChanged.disconnect();
          disableEvents();
        }
        if(value)
        {
          m_interfacePropertyChanged = value->propertyChanged.connect(std::bind_front(&CBUSBoosterDriver::interfacePropertyChanged, this));
        }
        return true;
      }}
  , node{this, "node", CBUS::NodeNumber::CANCMD, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript,
      [this](uint16_t /*value*/)
      {
        invalidateAll();
      }}
  , currentEvent{this, "current_event", 1, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript,
      [this](uint16_t /*value*/)
      {
        reportCurrent(); // invalidate
      }}
  , voltageEvent{this, "voltage_event", 2, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript,
      [this](uint16_t /*value*/)
      {
        reportVoltage(); // invalidate
      }}
{
  Attributes::addDisplayName(interface, DisplayName::Hardware::interface);
  Attributes::addEnabled(interface, false);
  Attributes::addObjectList(interface, getWorld(*this).cbusInterfaces);
  m_interfaceItems.add(interface);

  Attributes::addEnabled(node, false);
  m_interfaceItems.add(node);

  Attributes::addEnabled(currentEvent, false);
  m_interfaceItems.add(currentEvent);

  Attributes::addEnabled(voltageEvent, false);
  m_interfaceItems.add(voltageEvent);

  updateEnabled();
}

void CBUSBoosterDriver::destroying()
{
  disableEvents();
  m_interfacePropertyChanged.disconnect();
  BoosterDriver::destroying();
}

void CBUSBoosterDriver::loaded()
{
  BoosterDriver::loaded();

  if(interface)
  {
    m_interfacePropertyChanged = interface->propertyChanged.connect(std::bind_front(&CBUSBoosterDriver::interfacePropertyChanged, this));
  }
}

void CBUSBoosterDriver::worldEvent(WorldState state, WorldEvent event)
{
  BoosterDriver::worldEvent(state, event);
  switch(event)
  {
    case WorldEvent::EditDisabled:
    case WorldEvent::EditEnabled:
      updateEnabled();
      break;

    default:
      break;
  }
}

void CBUSBoosterDriver::interfaceOnlineChanged(bool online)
{
  if(online)
  {
    enableEvents();
  }
  else
  {
    disableEvents();
    invalidateAll();
  }
  updateEnabled();
}

void CBUSBoosterDriver::interfacePropertyChanged(BaseProperty& property)
{
  if(property.name() == "online")
  {
    interfaceOnlineChanged(static_cast<Property<bool>&>(property).value());
  }
}

void CBUSBoosterDriver::updateEnabled()
{
  const bool editable = contains(getWorld(*this).state, WorldState::Edit);
  const bool online = interface && interface->online;

  Attributes::setEnabled(interface, editable && !online);
  Attributes::setEnabled({node, currentEvent, voltageEvent}, editable);
}

void CBUSBoosterDriver::enableEvents()
{
  assert(interface);
  assert(interface->online);
  m_onReceiveHandle = interface->registerOnReceive(CBUS::OpCode::ACON2, std::bind_front(&CBUSBoosterDriver::receive, this));
}

void CBUSBoosterDriver::disableEvents()
{
  if(interface)
  {
    // unregister callback for received messages:
    interface->unregisterOnReceive(m_onReceiveHandle);
    m_onReceiveHandle = 0;
  }
}

void CBUSBoosterDriver::receive(uint8_t /*canId*/, const CBUS::Message& message)
{
  using namespace CBUS;

  assert(message.opCode == OpCode::ACON2); // enforced by filter
  const auto& acon2 = static_cast<const Accessory2On&>(message);

  if(acon2.nodeNumber() != node)
  {
    return;
  }

  if(acon2.eventNumber() == currentEvent)
  {
    reportCurrent(acon2.data() * currentScale);
  }
  else if(acon2.eventNumber() == voltageEvent)
  {
    reportVoltage(acon2.data() * voltageScale);
  }
}
