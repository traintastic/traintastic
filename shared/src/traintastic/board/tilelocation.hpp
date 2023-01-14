/**
 * server/src/board/tile/tilelocation.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020,2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_BOARD_TILELOCATION_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_BOARD_TILELOCATION_HPP

#include <cstdint>
#include <functional>
#include <limits>

struct TileLocation
{
  static const TileLocation invalid;

  int16_t x;
  int16_t y;

  bool isValid() const
  {
    return *this != invalid;
  }

  bool operator ==(const TileLocation& other) const
  {
    return x == other.x && y == other.y;
  }

  bool operator !=(const TileLocation& other) const
  {
    return x != other.x || y != other.y;
  }

  TileLocation adjusted(int16_t dx, int16_t dy)
  {
    return {static_cast<int16_t>(x + dx), static_cast<int16_t>(y + dy)};
  }
};

inline const TileLocation TileLocation::invalid{std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::min()};

struct TileLocationHash
{
  std::size_t operator()(const TileLocation& key) const
  {
    return std::hash<int16_t>()(key.x) ^ std::hash<int16_t>()(key.y);
  }
};

#endif
