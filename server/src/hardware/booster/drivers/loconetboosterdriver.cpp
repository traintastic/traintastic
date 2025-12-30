/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2025-2026 Reinder Feenstra
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

#include "loconetboosterdriver.hpp"
#include "../../interface/loconetinterface.hpp"
#include "../../../core/attributes.hpp"
#include "../../../core/objectproperty.tpp"
#include "../../../utils/displayname.hpp"
#include "../../../world/getworld.hpp"
#include "../../../world/world.hpp"

LocoNetBoosterDriver::LocoNetBoosterDriver(Booster& booster)
  : BoosterDriver(booster)
  , interface{this, "interface", nullptr, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript, nullptr,
      [this](std::shared_ptr<LocoNetInterface> value)
      {
        if(interface)
        {
          m_interfacePropertyChanged.disconnect();
        }
        if(value)
        {
          m_interfacePropertyChanged = value->propertyChanged.connect(std::bind_front(&LocoNetBoosterDriver::interfacePropertyChanged, this));
        }
        return true;
      }}
{
  Attributes::addDisplayName(interface, DisplayName::Hardware::interface);
  Attributes::addEnabled(interface, false);
  Attributes::addObjectList(interface, getWorld(*this).loconetInterfaces);
  m_interfaceItems.add(interface);
}

void LocoNetBoosterDriver::destroying()
{
  BoosterDriver::destroying();
  m_interfacePropertyChanged.disconnect();
}

void LocoNetBoosterDriver::loaded()
{
  BoosterDriver::loaded();

  if(interface)
  {
    m_interfacePropertyChanged = interface->propertyChanged.connect(std::bind_front(&LocoNetBoosterDriver::interfacePropertyChanged, this));
  }
}

void LocoNetBoosterDriver::worldEvent(WorldState state, WorldEvent event)
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

void LocoNetBoosterDriver::interfaceOnlineChanged(bool value)
{
  if(!value) // offline
  {
    invalidateAll();
  }
  updateEnabled();
}

void LocoNetBoosterDriver::interfacePropertyChanged(BaseProperty& property)
{
  if(property.name() == "online")
  {
    interfaceOnlineChanged(static_cast<Property<bool>&>(property).value());
  }
}

void LocoNetBoosterDriver::updateEnabled()
{
  const bool editable = contains(getWorld(*this).state, WorldState::Edit);
  const bool online = interface && interface->online;

  updateEnabled(editable, online);
}

void LocoNetBoosterDriver::updateEnabled(bool editable, bool online)
{
  Attributes::setEnabled(interface, editable && !online);
}
