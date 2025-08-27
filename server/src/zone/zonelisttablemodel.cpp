/**
 * server/src/zone/zonelisttablemodel.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#include "zonelisttablemodel.hpp"
#include "zone.hpp"
#include "zonelist.hpp"
#include "../core/objectproperty.tpp"
#include "../utils/displayname.hpp"

constexpr uint32_t columnId = 0;
constexpr uint32_t columnName = 1;

bool ZoneListTableModel::isListedProperty(std::string_view name)
{
  return
    name == "id" ||
    name == "name";
}

ZoneListTableModel::ZoneListTableModel(ObjectList<Zone>& list) :
  ObjectListTableModel<Zone>(list)
{
  setColumnHeaders({
    DisplayName::Object::id,
    DisplayName::Object::name,
    });
}

std::string ZoneListTableModel::getText(uint32_t column, uint32_t row) const
{
  if(row < rowCount())
  {
    const Zone& zone = getItem(row);

    switch(column)
    {
      case columnId:
        return zone.id;

      case columnName:
        return zone.name;

      default:
        assert(false);
        break;
    }
  }

  return "";
}

void ZoneListTableModel::propertyChanged(BaseProperty& property, uint32_t row)
{
  if(property.name() == "id")
    changed(row, columnId);
  else if(property.name() == "name")
    changed(row, columnName);
}
