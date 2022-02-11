/**
 * server/src/core/controllerlistbasetablemodel.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#include "controllerlistbasetablemodel.hpp"
#include "controllerlistbase.hpp"
#include "../utils/displayname.hpp"

constexpr uint32_t columnId = 0;
constexpr uint32_t columnName = 1;

bool ControllerListBaseTableModel::isListedProperty(std::string_view name)
{
  return
    name == "id" ||
    name == "name";
}

ControllerListBaseTableModel::ControllerListBaseTableModel(ControllerListBase& list)
  : m_list{list.shared_ptr<ControllerListBase>()}
{
  list.m_models.push_back(this);
  setRowCount(static_cast<uint32_t>(list.m_items.size()));

  setColumnHeaders({
    DisplayName::Object::id,
    DisplayName::Object::name,
    });
}

std::string ControllerListBaseTableModel::getText(uint32_t column, uint32_t row) const
{
  if(row < rowCount())
  {
    const Object& object = *m_list->m_items[row];

    switch(column)
    {
      case columnId:
        return object.getObjectId();

      case columnName:
        if(const auto* property = object.getProperty("name"))
          return property->toString();
        break;

      default:
        assert(false);
        break;
    }
  }

  return "";
}

void ControllerListBaseTableModel::propertyChanged(BaseProperty& property, uint32_t row)
{
  if(property.name() == "id")
    changed(row, columnId);
  else if(property.name() == "name")
    changed(row, columnName);
}
