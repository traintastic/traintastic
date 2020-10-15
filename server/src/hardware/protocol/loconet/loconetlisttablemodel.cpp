/**
 * server/src/hardware/input/loconetlisttablemodel.cpp
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

#include "loconetlisttablemodel.hpp"
#include "loconetlist.hpp"

constexpr uint32_t columnId = 0;

bool LocoNetListTableModel::isListedProperty(const std::string& name)
{
  return name == "id";
}

LocoNetListTableModel::LocoNetListTableModel(LocoNetList& list) :
  ObjectListTableModel<LocoNet::LocoNet>(list)
{
  setColumnHeaders({"id"});
}

std::string LocoNetListTableModel::getText(uint32_t column, uint32_t row) const
{
  if(row < rowCount())
  {
    const LocoNet::LocoNet& loconet = getItem(row);

    switch(column)
    {
      case columnId:
        return loconet.getObjectId();

      default:
        assert(false);
        break;
    }
  }

  return "";
}

void LocoNetListTableModel::propertyChanged(AbstractProperty& property, uint32_t row)
{
  if(property.name() == "id")
    changed(row, columnId);
}
