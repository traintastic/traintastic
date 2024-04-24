/**
 * shared/src/traintastic/enum/textalign.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_TEXTALIGN_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_TEXTALIGN_HPP

#include <cstdint>
#include <array>
#include "enum.hpp"

enum class TextAlign : uint8_t
{
  // Horizontal:
  Left = 0x01,
  HCenter = 0x02,
  Right = 0x04,
  // Vertical:
  Top = 0x10,
  VCenter = 0x20,
  Bottom = 0x40,
  // Combinations:
  TopLeft = Top | Left,
  TopCenter = Top | HCenter,
  TopRight = Top | Right,
  CenterLeft = VCenter | Left,
  Center = VCenter | HCenter,
  CenterRight = VCenter | Right,
  BottomLeft = Bottom | Left,
  BottomCenter = Bottom | HCenter,
  BottomRight = Bottom | Right,
};

TRAINTASTIC_ENUM(TextAlign, "text_align", 9,
{
  {TextAlign::TopLeft, "top_left"},
  {TextAlign::TopCenter, "top_center"},
  {TextAlign::TopRight, "top_right"},
  {TextAlign::CenterLeft, "center_left"},
  {TextAlign::Center, "center"},
  {TextAlign::CenterRight, "center_right"},
  {TextAlign::BottomLeft, "bottom_left"},
  {TextAlign::BottomCenter, "bottom_center"},
  {TextAlign::BottomRight, "bottom_right"},
});

inline constexpr std::array<TextAlign, 9> textAlignValues{{
  TextAlign::TopLeft,
  TextAlign::TopCenter,
  TextAlign::TopRight,
  TextAlign::CenterLeft,
  TextAlign::Center,
  TextAlign::CenterRight,
  TextAlign::BottomLeft,
  TextAlign::BottomCenter,
  TextAlign::BottomRight,
}};

#endif
