/**
 * server/src/train/trainvehiclelisttablemodel.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Filippo Gentile
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

#include "trainvehiclelisttablemodel.hpp"
#include "trainvehiclelist.hpp"
#include "trainvehiclelistitem.hpp"
#include "../vehicle/rail/railvehicle.hpp"
#include "../core/objectproperty.tpp"
#include "../utils/displayname.hpp"

constexpr uint32_t columnId = 0;
constexpr uint32_t columnVehicleId = 1;
constexpr uint32_t columnName = 2;
constexpr uint32_t columnType = 3;
constexpr uint32_t columnLOB = 4;
constexpr uint32_t columnInvertDir = 5;

bool TrainVehicleListTableModel::isListedProperty(std::string_view name)
{
  return
    name == "id" ||
    name == "name" ||
    name == "lob" ||
    name == "invert_direction";
}

TrainVehicleListTableModel::TrainVehicleListTableModel(TrainVehicleList& list)
  : TableModel()
  , m_list{list.shared_ptr<TrainVehicleList>()}
{
  list.m_models.push_back(this);
  setRowCount(static_cast<uint32_t>(list.items.size()));

  setColumnHeaders({
    DisplayName::Object::id,
    DisplayName::Object::id,
    DisplayName::Object::name,
    "rail_vehicle_list:type",
    DisplayName::Vehicle::Rail::lob,
    "train_vehicle_list_table_model:invert_direction"
  });
}

TrainVehicleListTableModel::~TrainVehicleListTableModel()
{
  auto it = std::find(m_list->m_models.begin(), m_list->m_models.end(), this);
  assert(it != m_list->m_models.end());
  m_list->m_models.erase(it);
}

std::string TrainVehicleListTableModel::getText(uint32_t column, uint32_t row) const
{
  if(row < rowCount())
  {
    const TrainVehicleListItem& item = *m_list->items[row];

    switch(column)
    {
      case columnId:
        return item.getObjectId();

      case columnVehicleId:
        return item.vehicle->id;

      case columnName:
        return item.vehicle->name;

      case columnType:
        return std::string("$class_id:").append(item.vehicle->getClassId()).append("$");

      case columnLOB:
        return toString(item.vehicle->lob);

      case columnInvertDir:
        return item.invertDirection ? "invert" : "normal";

      default:
        assert(false);
        break;
    }
  }

  return "";
}

void TrainVehicleListTableModel::propertyChanged(BaseProperty& property, uint32_t row)
{
  if(property.name() == "id")
    changed(row, columnId);
  else if(property.name() == "name")
    changed(row, columnName);
  else if(property.name() == "lob")
    changed(row, columnLOB);
  else if(property.name() == "invert_direction")
    changed(row, columnInvertDir);
}
