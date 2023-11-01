/**
 * server/src/board/tile/tile.cpp
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

#include "tile.hpp"
#include "../../core/attributes.hpp"
#include "../../core/objectproperty.tpp"
#include "../board.hpp"
#include "../boardlist.hpp"
#include "../../world/world.hpp"

Tile::Tile(World& world, std::string_view _id, TileId tileId)
  : IdObject(world, _id)
  , m_tileId{tileId}
  , x{this, "x", 0, PropertyFlags::ReadOnly | PropertyFlags::Store}
  , y{this, "y", 0, PropertyFlags::ReadOnly | PropertyFlags::Store}
  , rotate{this, "rotate", TileRotate::Deg0, PropertyFlags::ReadOnly | PropertyFlags::Store}
  , height{this, "height", 1, PropertyFlags::ReadOnly | PropertyFlags::Store}
  , width{this, "width", 1, PropertyFlags::ReadOnly | PropertyFlags::Store}
{
  Attributes::addObjectEditor(x, false);
  m_interfaceItems.add(x);

  Attributes::addObjectEditor(y, false);
  m_interfaceItems.add(y);

  Attributes::addObjectEditor(rotate, false);
  Attributes::addValues(rotate, tileRotateValues);
  m_interfaceItems.add(rotate);

  Attributes::addObjectEditor(height, false);
  Attributes::addMinMax<uint8_t>(height, 1, 1);
  m_interfaceItems.add(height);

  Attributes::addObjectEditor(width, false);
  Attributes::addMinMax<uint8_t>(width, 1, 1);
  m_interfaceItems.add(width);
}

Board& Tile::getBoard()
{
  for(const auto& board : *m_world.boards)
  {
    if(board->getTile(location()).get() == this)
    {
      return *board;
    }
  }
  assert(false);
  abort();
}

bool Tile::resize(uint8_t w, uint8_t h)
{
  assert(w >= 1);
  assert(h >= 1);

  if(w <= width.getAttribute<uint8_t>(AttributeName::Max) &&
      h <= height.getAttribute<uint8_t>(AttributeName::Max))
  {
    width.setValueInternal(w);
    height.setValueInternal(h);
    return true;
  }
  return false;
}
