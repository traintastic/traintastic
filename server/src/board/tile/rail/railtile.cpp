/**
 * server/src/board/tile/rail/railtile.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020,2023 Reinder Feenstra
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

#include "railtile.hpp"
#include "../../board.hpp"

RailTile::RailTile(World& world, std::string_view _id, TileId tileId) :
  Tile(world, _id, tileId)
{
}

void RailTile::reserve(uint8_t state)
{
  assert(state != 0);
  setReservedState(state);
}

void RailTile::setReservedState(uint8_t value)
{
  if(m_reservedState != value)
  {
    m_reservedState = value;
    auto& board = getBoard();
    board.tileDataChanged(board, location(), data());
  }
}
