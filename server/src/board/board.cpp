/**
 * server/src/board/board.cpp
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

#include "board.hpp"
#include "boardlisttablemodel.hpp"
#include "tile/tiles.hpp"
#include "../world/world.hpp"
#include "../core/attributes.hpp"

Board::Board(const std::weak_ptr<World>& world, std::string_view _id) :
  IdObject(world, _id),
  name{this, "name", "", PropertyFlags::ReadWrite | PropertyFlags::Store},
  addTile{*this, "add_tile",
    [this](int16_t x, int16_t y, TileRotate rotate, std::string_view classId)
    {
      const TileLocation l{x, y};
      auto w = m_world.lock();
      if(!w || m_tiles.find(l) != m_tiles.end())
        return false;
      auto tile = Tiles::create(w, classId);
      if(!tile)
        return false;
      tile->m_location = l;
      tile->m_data.setRotate(rotate);
      m_tiles[l] = tile;
      return true;
    }}
{
  auto w = world.lock();
  const bool editable = w && contains(w->state.value(), WorldState::Edit);

  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);
}

void Board::addToWorld()
{
  IdObject::addToWorld();

  if(auto world = m_world.lock())
    world->boards->addObject(shared_ptr<Board>());

}

void Board::worldEvent(WorldState state, WorldEvent event)
{
  IdObject::worldEvent(state, event);

  name.setAttributeEnabled(contains(state, WorldState::Edit));
}
