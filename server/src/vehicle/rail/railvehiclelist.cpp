/**
 * server/src/vehicle/rail/railvehiclelist.cpp
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

#include "railvehiclelist.hpp"
#include "railvehiclelisttablemodel.hpp"
#include "railvehicles.hpp"
#include "../../world/getworld.hpp"
#include "../../core/attributes.hpp"
#include "../../utils/displayname.hpp"

RailVehicleList::RailVehicleList(Object& _parent, std::string_view parentPropertyName) :
  ObjectList<RailVehicle>(_parent, parentPropertyName),
  add{*this, "add",
    [this](std::string_view railVehicleClassId)
    {
      auto& world = getWorld(parent());
      return RailVehicles::create(world, railVehicleClassId, world.getUniqueId("vehicle"));
    }}
  , remove{*this, "remove", std::bind(&RailVehicleList::removeMethodHandler, this, std::placeholders::_1)}
{
  const bool editable = contains(getWorld(parent()).state.value(), WorldState::Edit);

  Attributes::addDisplayName(add, DisplayName::List::add);
  Attributes::addEnabled(add, editable);
  Attributes::addClassList(add, RailVehicles::classList);
  m_interfaceItems.add(add);

  Attributes::addDisplayName(remove, DisplayName::List::remove);
  Attributes::addEnabled(remove, editable);
  m_interfaceItems.add(remove);
}

TableModelPtr RailVehicleList::getModel()
{
  return std::make_shared<RailVehicleListTableModel>(*this);
}

void RailVehicleList::worldEvent(WorldState state, WorldEvent event)
{
  ObjectList<RailVehicle>::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(add, editable);
  Attributes::setEnabled(remove, editable);
}

bool RailVehicleList::isListedProperty(std::string_view name)
{
  return RailVehicleListTableModel::isListedProperty(name);
}
