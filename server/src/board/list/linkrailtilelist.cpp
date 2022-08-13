/**
 * server/src/board/list/linkrailtilelist.cpp
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

#include "linkrailtilelist.hpp"
#include "linkrailtilelisttablemodel.hpp"

LinkRailTileList::LinkRailTileList(Object& _parent, std::string_view parentPropertyName)
  : ObjectList<LinkRailTile>(_parent, parentPropertyName)
{
}

TableModelPtr LinkRailTileList::getModel()
{
  return std::make_shared<LinkRailTileListTableModel>(*this);
}

bool LinkRailTileList::isListedProperty(std::string_view name)
{
  return LinkRailTileListTableModel::isListedProperty(name);
}
