/**
 * server/src/vehicle/rail/railvehiclelisttablemodel.cpp
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

#include "railvehiclelisttablemodel.hpp"
#include "railvehiclelist.hpp"
#include "../../utils/displayname.hpp"

constexpr uint32_t columnId = 0;
constexpr uint32_t columnName = 1;
constexpr uint32_t columnType = 2;
constexpr uint32_t columnLOB = 3;

bool RailVehicleListTableModel::isListedProperty(std::string_view name)
{
  return
    name == "id" ||
    name == "name" ||
    name == "lob";
}

RailVehicleListTableModel::RailVehicleListTableModel(ObjectList<RailVehicle>& list) :
  ObjectListTableModel<RailVehicle>(list)
{
  setColumnHeaders({
    DisplayName::Object::id,
    DisplayName::Object::name,
    "rail_vehicle_list:type",
    DisplayName::Vehicle::Rail::lob,
    });
}

std::string RailVehicleListTableModel::getText(uint32_t column, uint32_t row) const
{
  if(row < rowCount())
  {
    const RailVehicle& vehicle = getItem(row);

    switch(column)
    {
      case columnId:
        return vehicle.id;

      case columnName:
        return vehicle.name;

      case columnType:
        return std::string("$class_id:").append(vehicle.getClassId()).append("$");

      case columnLOB:
        return toString(vehicle.lob);

      default:
        assert(false);
        break;
    }
  }

  return "";
}

void RailVehicleListTableModel::propertyChanged(BaseProperty& property, uint32_t row)
{
  if(property.name() == "id")
    changed(row, columnId);
  else if(property.name() == "name")
    changed(row, columnName);
  else if(property.name() == "lob")
    changed(row, columnLOB);
}
