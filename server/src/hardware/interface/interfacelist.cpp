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
  create{*this, "create",
    [this](std::string_view interfaceClassId)
    {
      return Interfaces::create(getWorld(parent()), interfaceClassId);
    }},
  delete_{*this, "delete", std::bind(&InterfaceList::deleteMethodHandler, this, std::placeholders::_1)}
{
  const bool editable = contains(getWorld(parent()).state.value(), WorldState::Edit);

  Attributes::addDisplayName(create, DisplayName::List::create);
  Attributes::addEnabled(create, editable);
  Attributes::addClassList(create, Interfaces::classList);
  m_interfaceItems.add(create);

  Attributes::addDisplayName(delete_, DisplayName::List::delete_);
  Attributes::addEnabled(delete_, editable);
  m_interfaceItems.add(delete_);
}

TableModelPtr InterfaceList::getModel()
{
  return std::make_shared<InterfaceListTableModel>(*this);
}

void InterfaceList::worldEvent(WorldState state, WorldEvent event)
{
  ObjectList<Interface>::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(create, editable);
  Attributes::setEnabled(delete_, editable);
}

bool InterfaceList::isListedProperty(std::string_view name)
{
  return InterfaceListTableModel::isListedProperty(name);
}
