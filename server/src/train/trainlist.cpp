/**
 * server/src/vehicle/rail/trainlist.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021,2023 Reinder Feenstra
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
#include "../core/attributes.hpp"
#include "../utils/displayname.hpp"

TrainList::TrainList(Object& _parent, std::string_view parentPropertyName) :
  ObjectList<Train>(_parent, parentPropertyName),
  create{*this, "create",
    [this]()
    {
      auto& world = getWorld(parent());
      return Train::create(world, world.getUniqueId("train"));
    }}
  , delete_{*this, "delete", std::bind(&TrainList::deleteMethodHandler, this, std::placeholders::_1)}
{
  Attributes::addDisplayName(create, DisplayName::List::create);
  m_interfaceItems.add(create);

  Attributes::addDisplayName(delete_, DisplayName::List::delete_);
  m_interfaceItems.add(delete_);
}

TableModelPtr TrainList::getModel()
{
  return std::make_shared<TrainListTableModel>(*this);
}

bool TrainList::isListedProperty(std::string_view name)
{
  return TrainListTableModel::isListedProperty(name);
}
