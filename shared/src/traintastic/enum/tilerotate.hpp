/**
 * shared/src/traintastic/enum/tilerotate.hpp
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_TILEROTATE_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_TILEROTATE_HPP

#include <cstdint>
#include <type_traits>
#include "enum.hpp"

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

ENUM_NAME(TileRotate, "tile_rotate")

ENUM_VALUES(TileRotate, 8,
{
  {TileRotate::Deg0, "deg_0"},
  {TileRotate::Deg45, "deg_45"},
  {TileRotate::Deg90, "deg_90"},
  {TileRotate::Deg135, "deg_135"},
  {TileRotate::Deg180, "deg_180"},
  {TileRotate::Deg225, "deg_225"},
  {TileRotate::Deg270, "deg_270"},
  {TileRotate::Deg315, "deg_315"},
})

constexpr TileRotate operator +(TileRotate lhs, TileRotate rhs)
{
  return static_cast<TileRotate>((static_cast<std::underlying_type_t<TileRotate>>(lhs) + static_cast<std::underlying_type_t<TileRotate>>(rhs)) % 8);
}

constexpr TileRotate& operator +=(TileRotate& lhs, TileRotate rhs)
{
  lhs = lhs + rhs;
  return lhs;
}

constexpr TileRotate operator -(TileRotate lhs, TileRotate rhs)
{
  return static_cast<TileRotate>(((static_cast<std::underlying_type_t<TileRotate>>(lhs) + 8 - (static_cast<std::underlying_type_t<TileRotate>>(rhs)) % 8)) % 8);
}

constexpr TileRotate& operator -=(TileRotate& lhs, TileRotate rhs)
{
  lhs = lhs - rhs;
  return lhs;
}

constexpr bool isDiagonal(TileRotate value)
{
  return (static_cast<std::underlying_type_t<TileRotate>>(value) & 1);
}

inline TileRotate diff(TileRotate a, TileRotate b)
{
  return static_cast<TileRotate>(std::abs(static_cast<int8_t>(a) - static_cast<int8_t>(b)));
}

constexpr uint16_t toDeg(TileRotate value)
{
  return static_cast<uint16_t>(value) * 45;
}

constexpr TileRotate fromDeg(uint16_t value)
{
  return static_cast<TileRotate>((value / 45) % 8);
}

#endif
