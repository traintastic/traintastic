/**
 * client/src/enum/textalign.hpp
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

#ifndef TRAINTASTIC_CLIENT_ENUM_TEXTALIGN_HPP
#define TRAINTASTIC_CLIENT_ENUM_TEXTALIGN_HPP

#include <traintastic/enum/textalign.hpp>
#include <QtGlobal>

constexpr Qt::Alignment toAlignment(TextAlign value)
{
  switch(value)
  {
    case TextAlign::TopLeft:
      return Qt::AlignTop | Qt::AlignLeft;

    case TextAlign::TopCenter:
      return Qt::AlignTop | Qt::AlignHCenter;

    case TextAlign::TopRight:
      return Qt::AlignTop | Qt::AlignRight;

    case TextAlign::CenterLeft:
      return Qt::AlignVCenter | Qt::AlignLeft;

    case TextAlign::Center:
      return Qt::AlignCenter;

    case TextAlign::CenterRight:
      return Qt::AlignVCenter | Qt::AlignRight;

    case TextAlign::BottomLeft:
      return Qt::AlignBottom | Qt::AlignLeft;

    case TextAlign::BottomCenter:
      return Qt::AlignBottom | Qt::AlignHCenter;

    case TextAlign::BottomRight:
      return Qt::AlignBottom | Qt::AlignRight;

    default:
      break;
  }
  return Qt::Alignment();
}

#endif
