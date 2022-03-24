/**
 * client/src/enum/color.hpp
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

#ifndef TRAINTASTIC_CLIENT_ENUM_COLOR_HPP
#define TRAINTASTIC_CLIENT_ENUM_COLOR_HPP

#include <traintastic/enum/color.hpp>
#include <QColor>

inline QColor toQColor(Color color)
{
  switch(color)
  {
    case Color::None:
      return QColor();
    case Color::Black:
      return QColor(0,0,0);
    case Color::Silver:
      return QColor(192,192,192);
    case Color::Gray:
      return QColor(128,128,128);
    case Color::White:
      return QColor(255,255,255);
    case Color::Maroon:
      return QColor(128,0,0);
    case Color::Red:
      return QColor(255,0,0);
    case Color::Purple:
      return QColor(128,0,128);
    case Color::Fuchsia:
      return QColor(255,0,255);
    case Color::Green:
      return QColor(0,128,0);
    case Color::Lime:
      return QColor(0,255,0);
    case Color::Olive:
      return QColor(128,128,0);
    case Color::Yellow:
      return QColor(255,255,0);
    case Color::Navy:
      return QColor(0,0,128);
    case Color::Blue:
      return QColor(0,0,255);
    case Color::Teal:
      return QColor(0,128,128);
    case Color::Aqua:
      return QColor(0,255,255);
  }
  assert(false);
  return QColor();
}

#endif
