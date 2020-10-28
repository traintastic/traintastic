/**
 * server/src/hardware/output/outputlisttablemodel.cpp
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

#include "outputlisttablemodel.hpp"
#include "outputlist.hpp"
#include "loconetoutput.hpp"

constexpr uint32_t columnId = 0;
constexpr uint32_t columnName = 1;
constexpr uint32_t columnBus = 2;
constexpr uint32_t columnAddress = 3;

bool OutputListTableModel::isListedProperty(const std::string& name)
{
  return
    name == "id" ||
    name == "name";
}

OutputListTableModel::OutputListTableModel(OutputList& list) :
  ObjectListTableModel<Output>(list)
{
  setColumnHeaders({"Id", "Name", "output_list:bus", "output_list:address"});
}

std::string OutputListTableModel::getText(uint32_t column, uint32_t row) const
{
  if(row < rowCount())
  {
    const Output& output = getItem(row);
    const LocoNetOutput* outputLocoNet = dynamic_cast<const LocoNetOutput*>(&output);

    switch(column)
    {
      case columnId:
        return output.id;

      case columnName:
        return output.name;

      case columnBus: // virtual method @ Output ??
        if(outputLocoNet && outputLocoNet->loconet)
          return outputLocoNet->loconet->getObjectId();
        else
          return "";

      case columnAddress: // virtual method @ Output ??
        if(outputLocoNet)
          return std::to_string(outputLocoNet->address);
        else
          return "";

      default:
        assert(false);
        break;
    }
  }

  return "";
}

void OutputListTableModel::propertyChanged(AbstractProperty& property, uint32_t row)
{
  if(property.name() == "id")
    changed(row, columnId);
  else if(property.name() == "name")
    changed(row, columnName);
}
