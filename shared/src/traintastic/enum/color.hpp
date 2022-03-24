/**
 * shared/src/traintastic/enum/color.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_COLOR_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_COLOR_HPP

#include <cstdint>
#include "enum.hpp"

enum class Color : uint8_t
{
  None = 0,
  Black = 1,
  Silver = 2,
  Gray = 3,
  White = 4,
  Maroon = 5,
  Red = 6,
  Purple = 7,
  Fuchsia = 8,
  Green = 9,
  Lime = 10,
  Olive = 11,
  Yellow = 12,
  Navy = 13,
  Blue = 14,
  Teal = 15,
  Aqua = 16,
};

ENUM_NAME(Color, "color")

ENUM_VALUES(Color, 17,
{
  {Color::None, "none"},
  {Color::Black, "black"},
  {Color::Silver, "silver"},
  {Color::Gray, "gray"},
  {Color::White, "white"},
  {Color::Maroon, "maroon"},
  {Color::Red, "red"},
  {Color::Purple, "purple"},
  {Color::Fuchsia, "fuchsia"},
  {Color::Green, "green"},
  {Color::Lime, "lime"},
  {Color::Olive, "olive"},
  {Color::Yellow, "yellow"},
  {Color::Navy, "navy"},
  {Color::Blue, "blue"},
  {Color::Teal, "teal"},
  {Color::Aqua, "aqua"},
})

#endif
