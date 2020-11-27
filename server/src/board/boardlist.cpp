/**
 * server/src/board/boardlist.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020 Reinder Feenstra
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

BoardList::BoardList(Object& _parent, const std::string& parentPropertyName) :
  ObjectList<Board>(_parent, parentPropertyName),
  add{*this, "add",
    [this]()
    {
      auto world = getWorld(&this->parent());
      if(!world)
        return std::shared_ptr<Board>();
      return Board::create(world, world->getUniqueId("board"));
    }}
{
  auto world = getWorld(&_parent);
  const bool editable = world && contains(world->state.value(), WorldState::Edit);

  Attributes::addEnabled(add, editable);
  m_interfaceItems.add(add);
}

TableModelPtr BoardList::getModel()
{
  return std::make_shared<BoardListTableModel>(*this);
}

void BoardList::worldEvent(WorldState state, WorldEvent event)
{
  ObjectList<Board>::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  add.setAttributeEnabled(editable);
}

bool BoardList::isListedProperty(const std::string& name)
{
  return BoardListTableModel::isListedProperty(name);
}
