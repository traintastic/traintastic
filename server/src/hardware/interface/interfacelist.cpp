/**
 * server/src/hardware/interface/interfacelist.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023 Reinder Feenstra
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

#include "interfacelist.hpp"
#include "interfacelisttablemodel.hpp"
#include "interfaces.hpp"
#include "../../world/world.hpp"
#include "../../world/getworld.hpp"
#include "../../core/attributes.hpp"
#include "../../utils/displayname.hpp"

InterfaceList::InterfaceList(Object& _parent, std::string_view parentPropertyName) :
  ObjectList<Interface>(_parent, parentPropertyName),
  add{*this, "add",
    [this](std::string_view interfaceClassId)
    {
      return Interfaces::create(getWorld(parent()), interfaceClassId);
    }},
  remove{*this, "remove",
    [this](const std::shared_ptr<Interface>& object)
    {
      if(containsObject(object))
      {
#ifndef NDEBUG
        std::weak_ptr<Interface> weak = object;
#endif
        object->destroy(); // object might not be valid after this call!
        assert(weak.expired() || !containsObject(object));
      }
    }}
{
  const bool editable = contains(getWorld(parent()).state.value(), WorldState::Edit);

  Attributes::addDisplayName(add, DisplayName::List::add);
  Attributes::addEnabled(add, editable);
  Attributes::addClassList(add, Interfaces::classList);
  m_interfaceItems.add(add);

  Attributes::addDisplayName(remove, DisplayName::List::remove);
  Attributes::addEnabled(remove, editable);
  m_interfaceItems.add(remove);
}

TableModelPtr InterfaceList::getModel()
{
  return std::make_shared<InterfaceListTableModel>(*this);
}

void InterfaceList::worldEvent(WorldState state, WorldEvent event)
{
  ObjectList<Interface>::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(add, editable);
  Attributes::setEnabled(remove, editable);
}

bool InterfaceList::isListedProperty(std::string_view name)
{
  return InterfaceListTableModel::isListedProperty(name);
}
