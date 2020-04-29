/**
 * server/src/lua/scriptlisttablemodel.cpp
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

#include "scriptlisttablemodel.hpp"
#include "scriptlist.hpp"

namespace Lua {

constexpr uint32_t columnId = 0;
constexpr uint32_t columnName = 1;
constexpr uint32_t columnActive = 2;

bool ScriptListTableModel::isListedProperty(const std::string& name)
{
  return
    name == "id" ||
    name == "name" ||
    name == "active";
}

ScriptListTableModel::ScriptListTableModel(ScriptList& list) :
  ObjectListTableModel<Script>(list)
{
  setColumnHeaders({"lua_script:id", "lua_script:name", "lua_script:active"});
}

std::string ScriptListTableModel::getText(uint32_t column, uint32_t row) const
{
  if(row < rowCount())
  {
    const Script& script = getItem(row);

    switch(column)
    {
      case columnId:
        return script.id;

      case columnName:
        return script.name;

      case columnActive:
        return script.active  ? "\u2713" : "\u00D7";

      default:
        assert(false);
        break;
    }
  }

  return "";
}

void ScriptListTableModel::propertyChanged(AbstractProperty& property, uint32_t row)
{
  if(property.name() == "id")
    changed(row, columnId);
  else if(property.name() == "name")
    changed(row, columnName);
  else if(property.name() == "active")
    changed(row, columnActive);
}

};
