/**
 * server/src/board/board.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2021 Reinder Feenstra
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
    [this](int16_t x, int16_t y, TileRotate rotate, std::string_view classId, bool replace)
    {
      const TileLocation l{x, y};
      auto w = m_world.lock();
      if(!w)
        return false;

      if(auto it = m_tiles.find(l); it != m_tiles.end())
        if(!replace || !deleteTile(x, y))
          return false;

      auto tile = Tiles::create(w, classId);
      if(!tile)
        return false;

      tile->m_location = l;
      tile->setRotate(rotate);

      const int16_t x2 = tile->location().x + tile->data().width();
      const int16_t y2 = tile->location().y + tile->data().height();
      if(tile->location().x < sizeMin || x2 >= sizeMax ||
          tile->location().y < sizeMin || y2 >= sizeMax)
      {
        tile->destroy();
        return false;
      }
      for(int16_t x = tile->location().x; x < x2; x++)
        for(int16_t y = tile->location().y; y < y2; y++)
          m_tiles[TileLocation{x, y}] = tile;

      tileDataChanged(*this, tile->location(), tile->data());
      return true;
    }},
  deleteTile{*this, "delete_tile",
    [this](int16_t x, int16_t y)
    {
      const TileLocation l{x, y};
      auto it = m_tiles.find(l);
      if(it != m_tiles.end())
      {
        auto tile = it->second;
        const int16_t x2 = tile->location().x + tile->data().width();
        const int16_t y2 = tile->location().y + tile->data().height();
        for(int16_t x = tile->location().x; x < x2; x++)
          for(int16_t y = tile->location().y; y < y2; y++)
            m_tiles.erase(TileLocation{x, y});
        tileDataChanged(*this, tile->location(), TileData());
        tile->destroy();
      }
      return true;
    }}
{
  auto w = world.lock();
  const bool editable = w && contains(w->state.value(), WorldState::Edit);

  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);
  Attributes::addEnabled(addTile, editable);
  m_interfaceItems.add(addTile);
  Attributes::addEnabled(deleteTile, editable);
  m_interfaceItems.add(deleteTile);
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

  const bool editable = contains(state, WorldState::Edit);

  name.setAttributeEnabled(editable);
  addTile.setAttributeEnabled(editable);
  deleteTile.setAttributeEnabled(editable);
}
