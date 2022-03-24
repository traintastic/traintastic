/**
 * server/src/enum/color.hpp
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

#ifndef TRAINTASTIC_SERVER_ENUM_COLOR_HPP
#define TRAINTASTIC_SERVER_ENUM_COLOR_HPP

#include <traintastic/enum/color.hpp>
#include <array>

inline constexpr std::array<Color, 17> colorValues{{
  Color::None,
  Color::Black,
  Color::Silver,
  Color::Gray,
  Color::White,
  Color::Maroon,
  Color::Red,
  Color::Purple,
  Color::Fuchsia,
  Color::Green,
  Color::Lime,
  Color::Olive,
  Color::Yellow,
  Color::Navy,
  Color::Blue,
  Color::Teal,
  Color::Aqua,
}};

inline constexpr std::array<Color, 16> colorValuesWithoutNone{{
  Color::Black,
  Color::Silver,
  Color::Gray,
  Color::White,
  Color::Maroon,
  Color::Red,
  Color::Purple,
  Color::Fuchsia,
  Color::Green,
  Color::Lime,
  Color::Olive,
  Color::Yellow,
  Color::Navy,
  Color::Blue,
  Color::Teal,
  Color::Aqua,
}};

#endif
