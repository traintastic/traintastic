/**
 * server/src/board/boardlist.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2021,2023 Reinder Feenstra
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

#include "boardlist.hpp"
#include "boardlisttablemodel.hpp"
#include "../world/getworld.hpp"
#include "../core/attributes.hpp"
#include "../utils/displayname.hpp"

BoardList::BoardList(Object& _parent, std::string_view parentPropertyName) :
  ObjectList<Board>(_parent, parentPropertyName),
  add{*this, "add",
    [this]()
    {
      auto& world = getWorld(parent());
      return Board::create(world, world.getUniqueId("board"));
    }}
  , remove{*this, "remove", std::bind(&BoardList::removeMethodHandler, this, std::placeholders::_1)}
{
  const bool editable = contains(getWorld(parent()).state.value(), WorldState::Edit);

  Attributes::addDisplayName(add, DisplayName::List::add);
  Attributes::addEnabled(add, editable);
  m_interfaceItems.add(add);

  Attributes::addDisplayName(remove, DisplayName::List::remove);
  Attributes::addEnabled(remove, editable);
  m_interfaceItems.add(remove);
}

TableModelPtr BoardList::getModel()
{
  return std::make_shared<BoardListTableModel>(*this);
}

void BoardList::worldEvent(WorldState state, WorldEvent event)
{
  ObjectList<Board>::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(add, editable);
  Attributes::setEnabled(remove, editable);
}

bool BoardList::isListedProperty(std::string_view name)
{
  return BoardListTableModel::isListedProperty(name);
}
