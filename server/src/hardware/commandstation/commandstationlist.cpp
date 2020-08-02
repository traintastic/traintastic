/**
 * server/src/hardware/commandstation/commandstationlist.cpp
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

#include "commandstationlist.hpp"
#include "commandstationlisttablemodel.hpp"
#include "commandstations.hpp"
#include "../../world/world.hpp"
#include "../../world/getworld.hpp"

CommandStationList::CommandStationList(Object& _parent, const std::string& parentPropertyName) :
  ObjectList<CommandStation>(_parent, parentPropertyName),
  add{*this, "add",
    [this](std::string_view classId)
    {
      auto world = getWorld(this);
      if(!world)
        return std::shared_ptr<CommandStation>();
      return CommandStations::create(world, classId, world->getUniqueId("cs"));
    }},
  remove{*this, "remove",
    [this](const std::shared_ptr<CommandStation>& object)
    {
      if(containsObject(object))
        object->destroy();
      assert(!containsObject(object));
    }}
{
  auto world = getWorld(&_parent);
  const bool editable = world && contains(world->state.value(), WorldState::Edit);

  m_interfaceItems.add(add)
    .addAttributeEnabled(editable);
  m_interfaceItems.add(remove)
    .addAttributeEnabled(editable);
  }

TableModelPtr CommandStationList::getModel()
{
  return std::make_shared<CommandStationListTableModel>(*this);
}

void CommandStationList::worldEvent(WorldState state, WorldEvent event)
{
  ObjectList<CommandStation>::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  add.setAttributeEnabled(editable);
  remove.setAttributeEnabled(editable);
}

bool CommandStationList::isListedProperty(const std::string& name)
{
  return CommandStationListTableModel::isListedProperty(name);
}
