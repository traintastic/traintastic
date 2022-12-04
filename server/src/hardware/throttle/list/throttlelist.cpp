/**
 * server/src/hardware/throttle/list/throttlelist.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#include "throttlelist.hpp"
#include "throttlelisttablemodel.hpp"
#include "../throttlecontroller.hpp"

ThrottleList::ThrottleList(Object& _parent, std::string_view parentPropertyName, ThrottleListColumn _columns)
  : ObjectList<Throttle>(_parent, parentPropertyName)
  , columns{_columns}
{
}

TableModelPtr ThrottleList::getModel()
{
  return std::make_shared<ThrottleListTableModel>(*this);
}

bool ThrottleList::isListedProperty(std::string_view name)
{
  return ThrottleListTableModel::isListedProperty(name);
}
