/**
 * server/src/hardware/controller/controllerlist.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

#include "controllerlist.hpp"
#include "controllerlisttablemodel.hpp"
#include "controllers.hpp"
#include "../commandstation/commandstation.hpp"
#include "../../world/getworld.hpp"
#include "../../core/attributes.hpp"
#include "../../utils/displayname.hpp"

ControllerList::ControllerList(Object& _parent, const std::string& parentPropertyName) :
  ObjectList<Controller>(_parent, parentPropertyName),
  add{*this, "add",
    [this](std::string_view controllerClassId)
    {
      auto world = getWorld(&this->parent());
      if(!world)
        return std::shared_ptr<Controller>();
      auto controller = Controllers::create(world, controllerClassId, world->getUniqueId("controller"));
      if(auto* cs = dynamic_cast<CommandStation*>(&this->parent()))
        controller->commandStation = cs->shared_ptr<CommandStation>();
      //else if(world->commandStations->length() == 1)
      //  decoder->commandStation = cs->shared_ptr<CommandStation>();
      return controller;
    }}
  , remove{*this, "remove",
      [this](const std::shared_ptr<Controller>& controller)
      {
        if(containsObject(controller))
          controller->destroy();
        assert(!containsObject(controller));
      }}
{
  auto world = getWorld(&_parent);
  const bool editable = world && contains(world->state.value(), WorldState::Edit);

  Attributes::addDisplayName(add, DisplayName::List::add);
  Attributes::addEnabled(add, editable);
  Attributes::addClassList(add, Controllers::classList);
  m_interfaceItems.add(add);

  Attributes::addDisplayName(remove, DisplayName::List::remove);
  Attributes::addEnabled(remove, editable);
  m_interfaceItems.add(remove);
}

TableModelPtr ControllerList::getModel()
{
  return std::make_shared<ControllerListTableModel>(*this);
}

void ControllerList::worldEvent(WorldState state, WorldEvent event)
{
  ObjectList<Controller>::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(add, editable);
  Attributes::setEnabled(remove, editable);
}

bool ControllerList::isListedProperty(const std::string& name)
{
  return ControllerListTableModel::isListedProperty(name);
}
