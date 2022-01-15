/**
 * server/src/hardware/output/list/outputlisttablemodel.cpp
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

#include "outputlisttablemodel.hpp"
#include "outputlist.hpp"
#include "../../../utils/displayname.hpp"

bool OutputListTableModel::isListedProperty(std::string_view name)
{
  return
    name == "id" ||
    name == "name" ||
    name == "interface" ||
    name == "address";
}

OutputListTableModel::OutputListTableModel(OutputList& list) :
  ObjectListTableModel<Output>(list)
  , m_columnInterface(list.parentIsOutputController() ? invalidColumn : 2)
  , m_columnAddress(list.parentIsOutputController() ? 2 : 3)
{
  if(list.parentIsOutputController())
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

std::string OutputListTableModel::getText(uint32_t column, uint32_t row) const
{
  if(row < rowCount())
  {
    const Output& output = getItem(row);

    if(column == columnId)
      return output.id;
    if(column == columnName)
      return output.name;
    if(column == m_columnInterface)
    {
        if(const auto& interface = std::dynamic_pointer_cast<Object>(output.interface.value()))
        {
          if(auto* property = interface->getProperty("name"); property && !property->toString().empty())
            return property->toString();

          return interface->getObjectId();
        }
        return "";
    }
    if(column == m_columnAddress)
      return std::to_string(output.address.value());
    assert(false);
  }

  return "";
}

void OutputListTableModel::propertyChanged(BaseProperty& property, uint32_t row)
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
