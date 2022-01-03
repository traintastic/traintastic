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
#include "../../../utils/displayname.hpp"

bool InputListTableModel::isListedProperty(const std::string& name)
{
  return
    name == "id" ||
    name == "name" ||
    name == "interface" ||
    name == "address";
}

InputListTableModel::InputListTableModel(InputList& list)
  : ObjectListTableModel<Input>(list)
  , m_columnInterface(list.parentIsInputController() ? invalidColumn : 2)
  , m_columnAddress(list.parentIsInputController() ? 2 : 3)
{
  if(list.parentIsInputController())
  {
    setColumnHeaders({
      DisplayName::Object::id,
      DisplayName::Object::name,
      DisplayName::Hardware::address,
      });
  }
  else
  {
    setColumnHeaders({
      DisplayName::Object::id,
      DisplayName::Object::name,
      DisplayName::Hardware::interface,
      DisplayName::Hardware::address,
      });
  }
}

std::string InputListTableModel::getText(uint32_t column, uint32_t row) const
{
  if(row < rowCount())
  {
    const Input& input = getItem(row);

    if(column == columnId)
      return input.id;
    if(column == columnName)
      return input.name;
    if(column == m_columnInterface)
    {
      if(const auto& interface = std::dynamic_pointer_cast<Object>(input.interface.value()))
      {
        if(auto* property = interface->getProperty("name"); property && !property->toString().empty())
          return property->toString();

        return interface->getObjectId();
      }
      return "";
    }
    if(column == m_columnAddress)
      return std::to_string(input.address.value());

    assert(false);
  }

  return "";
}

void InputListTableModel::propertyChanged(BaseProperty& property, uint32_t row)
{
  if(property.name() == "id")
    changed(row, columnId);
  else if(property.name() == "name")
    changed(row, columnName);
  else if(property.name() == "interface" && m_columnInterface != invalidColumn)
    changed(row, m_columnInterface);
  else if(property.name() == "address")
    changed(row, m_columnAddress);
  }
