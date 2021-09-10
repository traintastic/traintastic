/**
 * server/src/hardware/input/list/inputlisttablemodel.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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

#include "inputlisttablemodel.hpp"
#include "inputlist.hpp"
#include "../loconetinput.hpp"
#include "../../../utils/displayname.hpp"

constexpr uint32_t columnId = 0;
constexpr uint32_t columnName = 1;
constexpr uint32_t columnBus = 2;
constexpr uint32_t columnAddress = 3;

bool InputListTableModel::isListedProperty(const std::string& name)
{
  return
    name == "id" ||
    name == "name";
}

InputListTableModel::InputListTableModel(InputList& list) :
  ObjectListTableModel<Input>(list)
{
  setColumnHeaders({
    DisplayName::Object::id,
    DisplayName::Object::name,
    "input_list:bus",
    DisplayName::Hardware::address,
    });
}

std::string InputListTableModel::getText(uint32_t column, uint32_t row) const
{
  if(row < rowCount())
  {
    const Input& input = getItem(row);
    const LocoNetInput* inputLocoNet = dynamic_cast<const LocoNetInput*>(&input);

    switch(column)
    {
      case columnId:
        return input.id;

      case columnName:
        return input.name;

      case columnBus: // virtual method @ Input ??
        if(inputLocoNet && inputLocoNet->loconet)
          return inputLocoNet->loconet->getObjectId();
        else
          return "";

      case columnAddress: // virtual method @ Input ??
        if(inputLocoNet)
          return std::to_string(inputLocoNet->address);
        else
          return "";

      default:
        assert(false);
        break;
    }
  }

  return "";
}

void InputListTableModel::propertyChanged(BaseProperty& property, uint32_t row)
{
  if(property.name() == "id")
    changed(row, columnId);
  else if(property.name() == "name")
    changed(row, columnName);
}
