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

#include "board.hpp"
#include "boardlist.hpp"
#include "boardlisttablemodel.hpp"
#include "../core/method.tpp"
#include "../world/getworld.hpp"
#include "../core/attributes.hpp"
#include "../utils/displayname.hpp"
#ifdef ENABLE_LOG_DEBUG
  #include "../log/log.hpp"
#endif

BoardList::BoardList(Object& _parent, std::string_view parentPropertyName) :
  ObjectList<Board>(_parent, parentPropertyName),
  create{*this, "create",
    [this]()
    {
      auto& world = getWorld(parent());
      return Board::create(world, world.getUniqueId("board"));
    }}
  , delete_{*this, "delete", std::bind(&BoardList::deleteMethodHandler, this, std::placeholders::_1)}
{
  const bool editable = contains(getWorld(parent()).state.value(), WorldState::Edit);

  Attributes::addDisplayName(create, DisplayName::List::create);
  Attributes::addEnabled(create, editable);
  m_interfaceItems.add(create);

  Attributes::addDisplayName(delete_, DisplayName::List::remove);
  Attributes::addEnabled(delete_, editable);
  m_interfaceItems.add(delete_);
}

TableModelPtr BoardList::getModel()
{
  return std::make_shared<BoardListTableModel>(*this);
}

void BoardList::worldEvent(WorldState state, WorldEvent event)
{
  ObjectList<Board>::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(create, editable);
  Attributes::setEnabled(delete_, editable);

  // handle board modified:
  if(event == WorldEvent::EditDisabled || event == WorldEvent::Run)
  {
#ifdef ENABLE_LOG_DEBUG
    const auto start = std::chrono::steady_clock::now();
#endif

    // due to link tile a change on one board might require an update on another board.
    // for now update all board if one board is changed (should be improved!)
    bool boardModified = false;
    for(auto& board : m_items)
    {
      if(board->m_modified)
      {
        boardModified = true;
        break;
      }
    }
    if(boardModified)
    {
      for(auto& board : m_items)
      {
        board->m_modified = true;
        board->modified();
      }
    }

#ifdef ENABLE_LOG_DEBUG
  const auto duration = std::chrono::steady_clock::now() - start;
  Log::debug("Board modified took", std::chrono::duration_cast<std::chrono::microseconds>(duration).count() / 1e3 , "ms");
#endif
  }
}

bool BoardList::isListedProperty(std::string_view name)
{
  return BoardListTableModel::isListedProperty(name);
}
