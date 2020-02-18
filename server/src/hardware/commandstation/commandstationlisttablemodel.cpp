/**
 * server/src/core/
 *
 * This file is part of the traintastic source code
 *
 * Copyright (C) 2019 Reinder Feenstra
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

#include "commandstationlisttablemodel.hpp"
#include "commandstationlist.hpp"

using Hardware::CommandStation::CommandStation;

constexpr uint32_t columnId = 0;
constexpr uint32_t columnName = 1;
constexpr uint32_t columnOnline = 2;
constexpr uint32_t columnStatus = 3;

bool CommandStationListTableModel::isListedProperty(const std::string& name)
{
  return
    name == "id" ||
    name == "name" ||
    name == "online" ||
    name == "status";
}

CommandStationListTableModel::CommandStationListTableModel(CommandStationList& list) :
  ObjectListTableModel<CommandStation>(list)
{
  setColumnHeaders({"Id", "Name", "Online", "Status"});
}

std::string CommandStationListTableModel::getText(uint32_t column, uint32_t row) const
{
  if(row < rowCount())
  {
    const CommandStation& cs = getItem(row);

    switch(column)
    {
      case columnId:
        return cs.id;

      case columnName:
        return cs.name;

      case columnOnline:
        return cs.online ? "\u2022" : "";

      case columnStatus:
        return "?";

      default:
        assert(false);
        break;
    }
  }

  return "";
}

void CommandStationListTableModel::propertyChanged(AbstractProperty& property, uint32_t row)
{
  if(property.name() == "id")
    changed(row, columnId);
  else if(property.name() == "name")
    changed(row, columnName);
  else if(property.name() == "online")
    changed(row, columnOnline);
  else if(property.name() == "status")
    changed(row, columnStatus);
}
