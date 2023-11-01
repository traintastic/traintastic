/**
 * client/src/board/boardcolorscheme.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_BOARD_BOARDCOLORSCHEME_HPP
#define TRAINTASTIC_CLIENT_BOARD_BOARDCOLORSCHEME_HPP

#include <QColor>

struct BoardColorScheme
{
  static const BoardColorScheme dark;
  static const BoardColorScheme light;

  const QColor background;
  const QColor foreground;
  const QColor track;
  const QColor trackDisabled;
  const QColor trackReserved;
  const QColor trackReservedDisabled;
  const QColor blockFree;
  const QColor blockReserved;
  const QColor blockOccupied;
  const QColor blockUnknown;
  const QColor blockText;
  const QColor sensorFree;
  const QColor sensorOccupied;
  const QColor sensorIdle;
  const QColor sensorTriggered;
  const QColor sensorUnknown;
  const QColor turnoutState;
  const QColor decouplerDeactivated;
  const QColor decouplerActivated;
};

#endif
