/**
 * server/src/vehicle/rail/trainlist.cpp
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

#include "trainlist.hpp"
#include "trainlisttablemodel.hpp"
#include "../world/world.hpp"
#include "../world/getworld.hpp"

TrainList::TrainList(Object& _parent, const std::string& parentPropertyName) :
  ObjectList<Train>(_parent, parentPropertyName),
  add{*this, "add",
    [this]()
    {
      auto world = getWorld(&this->parent());
      if(!world)
        return std::shared_ptr<Train>();
      return Train::create(world, world->getUniqueId("train"));
    }}
{
  auto world = getWorld(&_parent);
  const bool editable = world && contains(world->state.value(), WorldState::Edit);

  m_interfaceItems.add(add)
    .addAttributeEnabled(editable);
}

TableModelPtr TrainList::getModel()
{
  return std::make_shared<TrainListTableModel>(*this);
}

void TrainList::worldEvent(WorldState state, WorldEvent event)
{
  ObjectList<Train>::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  add.setAttributeEnabled(editable);
}

bool TrainList::isListedProperty(const std::string& name)
{
  return TrainListTableModel::isListedProperty(name);
}
