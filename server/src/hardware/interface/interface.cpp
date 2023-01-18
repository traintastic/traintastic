/**
 * server/src/hardware/interface/interface.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
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

#include "interface.hpp"
#include "interfacelisttablemodel.hpp"
#include "../../core/attributes.hpp"
#include "../../utils/displayname.hpp"
#include "../../world/world.hpp"

Interface::Interface(World& world, std::string_view _id)
  : IdObject(world, _id)
  , name{this, "name", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , online{this, "online", false, PropertyFlags::ReadWrite | PropertyFlags::NoStore, nullptr,
      [this](bool& value)
      {
        return setOnline(value, contains(m_world.state.value(), WorldState::Simulation));
      }}
  , status{this, "status", InterfaceStatus::Offline, PropertyFlags::ReadOnly | PropertyFlags::NoStore}
  , notes{this, "notes", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  const bool editable = contains(m_world.state.value(), WorldState::Edit);

  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);

  Attributes::addDisplayName(online, DisplayName::Interface::online);
  Attributes::addEnabled(online, contains(m_world.state.value(), WorldState::Online));
  m_interfaceItems.add(online);

  Attributes::addDisplayName(status, DisplayName::Interface::status);
  Attributes::addObjectEditor(status, false);
  Attributes::addValues(status, interfaceStatusValues);
  m_interfaceItems.add(status);

  Attributes::addDisplayName(notes, DisplayName::Object::notes);
  m_interfaceItems.add(notes);
}

void Interface::addToWorld()
{
  IdObject::addToWorld();
  m_world.interfaces->addObject(shared_ptr<Interface>());
}

void Interface::destroying()
{
  m_world.interfaces->removeObject(shared_ptr<Interface>());
  IdObject::destroying();
}

void Interface::worldEvent(WorldState state, WorldEvent event)
{
  IdObject::worldEvent(state, event);

  Attributes::setEnabled(name, contains(state, WorldState::Edit));

  switch(event)
  {
    case WorldEvent::Offline:
      online = false;
      Attributes::setEnabled(online, false);
      break;

    case WorldEvent::Online:
      Attributes::setEnabled(online, true);
      try
      {
        online = true;
      }
      catch(const invalid_value_error&)
      {
      }
      break;

    default:
      break;
  }
}
