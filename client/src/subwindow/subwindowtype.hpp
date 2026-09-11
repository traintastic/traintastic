/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2021-2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_SUBWINDOW_SUBWINDOWTYPE_HPP
#define TRAINTASTIC_CLIENT_SUBWINDOW_SUBWINDOWTYPE_HPP

#include <optional>
#include <QString>

enum class SubWindowType
{
  Object,
  Board,
  Throttle,
};

inline QString toString(SubWindowType value)
{
  switch(value)
  {
    case SubWindowType::Object:
      return QStringLiteral("object");

    case SubWindowType::Board:
      return QStringLiteral("board");

    case SubWindowType::Throttle:
      return QStringLiteral("throttle");
  }
  Q_ASSERT(false);
  return QString();
}

inline std::optional<SubWindowType> toSubWindowType(const QString& value)
{
  if(value == "object")
  {
    return SubWindowType::Object;
  }
  if(value == "board")
  {
    return SubWindowType::Board;
  }
  if(value == "throttle")
  {
    return SubWindowType::Throttle;
  }
  return std::nullopt;
}

#endif
