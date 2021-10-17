/**
 * server/src/hardware/interface/interface.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

Interface::Interface(const std::weak_ptr<World>& world, std::string_view _id)
  : IdObject(world, _id)
  , name{this, "name", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , online{this, "online", false, PropertyFlags::ReadWrite | PropertyFlags::NoStore, nullptr, std::bind(&Interface::setOnline, this, std::placeholders::_1)}
  , status{this, "status", InterfaceStatus::Offline, PropertyFlags::ReadOnly | PropertyFlags::NoStore}
  , notes{this, "notes", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  auto w = world.lock();
  const bool editable = w && contains(w->state.value(), WorldState::Edit);

  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);

  Attributes::addDisplayName(online, DisplayName::CommandStation::online);
  m_interfaceItems.add(online);

  Attributes::addDisplayName(status, DisplayName::Interface::status);
  Attributes::addValues(status, interfaceStatusValues);
  m_interfaceItems.add(status);

  Attributes::addDisplayName(notes, DisplayName::Object::notes);
  m_interfaceItems.add(notes);
}

void Interface::addToWorld()
{
  IdObject::addToWorld();

  if(auto world = m_world.lock())
    world->interfaces->addObject(shared_ptr<Interface>());
}

void Interface::destroying()
{
  if(auto world = m_world.lock())
    world->interfaces->removeObject(shared_ptr<Interface>());
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
      break;

    case WorldEvent::Online:
      online = true;
      break;

    default:
      break;
  }
}
