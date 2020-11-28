/**
 * shared/src/traintastic/board/tilerotate.hpp
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_BOARD_TILEROTATE_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_BOARD_TILEROTATE_HPP

#include <cstdint>
#include <type_traits>

enum class TileRotate : uint8_t // 3 bit
{
  Deg0 = 0,
  Deg45 = 1,
  Deg90 = 2,
  Deg135 = 3,
  Deg180 = 4,
  Deg225 = 5,
  Deg270 = 6,
  Deg315 = 7
};

constexpr TileRotate operator +(TileRotate lhs, TileRotate rhs)
{
  return static_cast<TileRotate>((static_cast<std::underlying_type_t<TileRotate>>(lhs) + static_cast<std::underlying_type_t<TileRotate>>(rhs)) % 8);
}

constexpr TileRotate operator -(TileRotate lhs, TileRotate rhs)
{
  return static_cast<TileRotate>((static_cast<std::underlying_type_t<TileRotate>>(lhs) + 8 - static_cast<std::underlying_type_t<TileRotate>>(rhs)) % 8);
}

constexpr bool isDiagonal(TileRotate value)
{
  return (static_cast<std::underlying_type_t<TileRotate>>(value) & 1);
}

#endif
