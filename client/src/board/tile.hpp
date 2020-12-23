/**
 * client/src/board/tile.cpp
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

#ifndef TRAINTASTIC_CLIENT_BOARD_TILE_HPP
#define TRAINTASTIC_CLIENT_BOARD_TILE_HPP

#include <QPainter>
#include <traintastic/board/tilerotate.hpp>

struct Tile
{
  inline static const QColor signalRed{192, 0, 0};
  inline static const QColor signalYellow{192, 192 , 32};
  inline static const QColor signalGreen{0, 192, 0};

  static void drawStraight(QPainter& painter, const QRectF& r, TileRotate rotate);
  static void drawCurve45(QPainter& painter, const QRectF& r, TileRotate rotate);
  static void drawCurve90(QPainter& painter, QRectF r, TileRotate rotate);

  static void drawBufferStop(QPainter& painter, const QRectF& r, TileRotate rotate);

  static void drawSignal2Aspect(QPainter& painter, QRectF r, TileRotate rotate);
  static void drawSignal3Aspect(QPainter& painter, QRectF r, TileRotate rotate);
};

#endif
