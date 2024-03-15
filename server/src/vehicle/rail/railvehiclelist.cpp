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
#include "../../core/objectproperty.tpp"
#include "../../core/method.tpp"
#include "../../utils/displayname.hpp"
#include "../../log/logmessageexception.hpp"
#include "../../train/train.hpp"
#include "../../hardware/decoder/decoder.hpp"

RailVehicleList::RailVehicleList(Object& _parent, std::string_view parentPropertyName) :
  ObjectList<RailVehicle>(_parent, parentPropertyName),
  create{*this, "create",
    [this](std::string_view railVehicleClassId)
    {
      auto& world = getWorld(parent());
      return RailVehicles::create(world, railVehicleClassId, world.getUniqueId("vehicle"));
    }}
  , delete_{*this, "delete",
      [this](const std::shared_ptr<RailVehicle>& railVehicle)
      {
        if(railVehicle->activeTrain.value())
        {
          throw LogMessageException(LogMessage::E3001_CANT_DELETE_RAIL_VEHICLE_WHEN_IN_ACTIVE_TRAIN);
        }
        RailVehicleList::deleteMethodHandler(railVehicle);
      }}
{
  const bool editable = contains(getWorld(parent()).state.value(), WorldState::Edit);

  Attributes::addDisplayName(create, DisplayName::List::create);
  Attributes::addEnabled(create, editable);
  Attributes::addClassList(create, RailVehicles::classList);
  m_interfaceItems.add(create);

  Attributes::addDisplayName(delete_, DisplayName::List::delete_);
  Attributes::addEnabled(delete_, editable);
  m_interfaceItems.add(delete_);
}

TableModelPtr RailVehicleList::getModel()
{
  return std::make_shared<RailVehicleListTableModel>(*this);
}

void RailVehicleList::worldEvent(WorldState state, WorldEvent event)
{
  ObjectList<RailVehicle>::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(create, editable);
  Attributes::setEnabled(delete_, editable);
}

bool RailVehicleList::isListedProperty(std::string_view name)
{
  return RailVehicleListTableModel::isListedProperty(name);
}
