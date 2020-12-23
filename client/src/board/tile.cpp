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

#include "tile.hpp"
#include <cmath>
#include "../utils/rectf.hpp"

void Tile::drawStraight(QPainter& painter, const QRectF& r, TileRotate rotate)
{
  switch(rotate)
  {
    case TileRotate::Deg0:
    case TileRotate::Deg180:
      painter.drawLine(bottomCenter(r), topCenter(r));
      break;

    case TileRotate::Deg45:
    case TileRotate::Deg225:
      painter.drawLine(r.bottomLeft(), r.topRight());
      break;

    case TileRotate::Deg90:
    case TileRotate::Deg270:
      painter.drawLine(centerLeft(r), centerRight(r));
      break;

    case TileRotate::Deg135:
    case TileRotate::Deg315:
      painter.drawLine(r.bottomRight(), r.topLeft());
      break;
  }
}

void Tile::drawCurve45(QPainter& painter, const QRectF& r, TileRotate rotate)
{
  QPainterPath path;
  switch(rotate)
  {
    case TileRotate::Deg0:
      path.moveTo(bottomCenter(r));
      path.quadTo(r.center(), r.topLeft());
      break;

    case TileRotate::Deg45:
      path.moveTo(r.bottomLeft());
      path.quadTo(r.center(), topCenter(r));
      break;

    case TileRotate::Deg90:
      path.moveTo(centerLeft(r));
      path.quadTo(r.center(), r.topRight());
      break;

    case TileRotate::Deg135:
      path.moveTo(r.topLeft());
      path.quadTo(r.center(), centerRight(r));
      break;

    case TileRotate::Deg180:
      path.moveTo(topCenter(r));
      path.quadTo(r.center(), r.bottomRight());
      break;

    case TileRotate::Deg225:
      path.moveTo(r.topRight());
      path.quadTo(r.center(), bottomCenter(r));
      break;

    case TileRotate::Deg270:
      path.moveTo(centerRight(r));
      path.quadTo(r.center(), r.bottomLeft());
      break;

    case TileRotate::Deg315:
      path.moveTo(r.bottomRight());
      path.quadTo(r.center(), centerLeft(r));
      break;
  }
  painter.drawPath(path);
}

void Tile::drawCurve90(QPainter& painter, QRectF r, TileRotate rotate)
{
  const int spanAngle = 90 * 16;
  const qreal size = r.width();

  if(isDiagonal(rotate))
  {
    const qreal grow = size * 0.5 * (std::sqrt(2) - 1);
    r.adjust(-grow, -grow, grow, grow);
  }

  switch(rotate)
  {
    case TileRotate::Deg0:
      r.translate(-size * 0.5, size * 0.5);
      painter.drawArc(r, 0 * 16, spanAngle);
      break;

    case TileRotate::Deg45:
      r.translate(-size, 0);
      painter.drawArc(r, 315 * 16, spanAngle);
      break;

    case TileRotate::Deg90:
      r.translate(-size * 0.5, -size * 0.5);
      painter.drawArc(r, 270 * 16, spanAngle);
      break;

    case TileRotate::Deg135:
      r.translate(0, -size);
      painter.drawArc(r, 225 * 16, spanAngle);
      break;

    case TileRotate::Deg180:
      r.translate(size * 0.5, -size * 0.5);
      painter.drawArc(r, 180 * 16, spanAngle);
      break;

    case TileRotate::Deg225:
      r.translate(size, 0);
      painter.drawArc(r, 135 * 16, spanAngle);
      break;

    case TileRotate::Deg270:
      r.translate(size * 0.5, size * 0.5);
      painter.drawArc(r, 90 * 16, spanAngle);
      break;

    case TileRotate::Deg315:
      r.translate(0, size);
      painter.drawArc(r, 45 * 16, spanAngle);
      break;
  }
}

void Tile::drawBufferStop(QPainter& painter, const QRectF& r, TileRotate rotate)
{
  const qreal stopSize = 0.5 + qRound(r.width() / 4);
  const qreal stopSizeDiagonal = stopSize / std::sqrt(2);
  const qreal cx = r.center().x();
  const qreal cy = r.center().y();

  switch(rotate)
  {
    case TileRotate::Deg0:
      painter.drawLine(bottomCenter(r), r.center());
      painter.drawLine(QPointF{cx - stopSize, cy}, QPointF{cx + stopSize, cy});
      break;

    case TileRotate::Deg45:
      painter.drawLine(r.bottomLeft(), r.center());
      painter.drawLine(QPointF{cx - stopSizeDiagonal, cy - stopSizeDiagonal}, QPointF{cx + stopSizeDiagonal, cy + stopSizeDiagonal});
      break;

    case TileRotate::Deg90:
      painter.drawLine(centerLeft(r), r.center());
      painter.drawLine(QPointF{cx, cy - stopSize}, QPointF{cx, cy + stopSize});
      break;

    case TileRotate::Deg135:
      painter.drawLine(r.topLeft(), r.center());
      painter.drawLine(QPointF{cx - stopSizeDiagonal, cy + stopSizeDiagonal}, QPointF{cx + stopSizeDiagonal, cy - stopSizeDiagonal});
      break;

    case TileRotate::Deg180:
      painter.drawLine(topCenter(r), r.center());
      painter.drawLine(QPointF{cx - stopSize, cy}, QPointF{cx + stopSize, cy});
      break;

    case TileRotate::Deg225:
      painter.drawLine(r.topRight(), r.center());
      painter.drawLine(QPointF{cx - stopSizeDiagonal, cy - stopSizeDiagonal}, QPointF{cx + stopSizeDiagonal, cy + stopSizeDiagonal});
      break;

    case TileRotate::Deg270:
      painter.drawLine(centerRight(r), r.center());
      painter.drawLine(QPointF{cx, cy - stopSize}, QPointF{cx, cy + stopSize});
      break;

    case TileRotate::Deg315:
      painter.drawLine(r.bottomRight(), r.center());
      painter.drawLine(QPointF{cx - stopSizeDiagonal, cy + stopSizeDiagonal}, QPointF{cx + stopSizeDiagonal, cy - stopSizeDiagonal});
      break;
  }
}

void Tile::drawSignal2Aspect(QPainter& painter, QRectF r, TileRotate rotate)
{
  painter.save();
  painter.translate(r.center());
  painter.rotate(toDeg(rotate));
  r.moveCenter(QPointF{0, 0});

  const qreal x1 = r.left() + r.width() * 0.3;
  const qreal x2 = r.right() - r.width() * 0.3;
  const qreal y1 = r.top() + r.height() * 0.4;
  const qreal y2 = r.bottom() - r.height() * 0.4;
  const qreal w = x2 - x1;
  const qreal lampRadius = w / 4;

  QPainterPath path;
  path.moveTo(x1, y1);
  path.lineTo(x1, y2);
  path.arcTo(QRectF{x1, y2 - (w / 2), w, w}, 180, 180);
  path.lineTo(x2, y1);
  path.arcTo(QRectF{x1, y1 - (w / 2), w, w}, 0, 180);
  painter.setPen(QPen{Qt::white, lampRadius / 2});
  painter.fillPath(path, Qt::black);
  painter.drawPath(path);

  painter.setPen(Qt::NoPen);

  painter.setBrush(signalRed);
  painter.drawEllipse(QPointF{r.center().x(), r.center().y() - lampRadius}, lampRadius, lampRadius);

  painter.setBrush(signalGreen);
  painter.drawEllipse(QPointF{r.center().x(), r.center().y() + lampRadius}, lampRadius, lampRadius);

  painter.restore();
}

void Tile::drawSignal3Aspect(QPainter& painter, QRectF r, TileRotate rotate)
{
  painter.save();
  painter.translate(r.center());
  painter.rotate(toDeg(rotate));
  r.moveCenter(QPointF{0, 0});

  const qreal x1 = r.left() + r.width() * 0.3;
  const qreal x2 = r.right() - r.width() * 0.3;
  const qreal y1 = r.top() + r.height() * 0.3;
  const qreal y2 = r.bottom() - r.height() * 0.3;
  const qreal w = x2 - x1;
  const qreal lampRadius = w / 4;

  QPainterPath path;
  path.moveTo(x1, y1);
  path.lineTo(x1, y2);
  path.arcTo(QRectF{x1, y2 - (w / 2), w, w}, 180, 180);
  path.lineTo(x2, y1);
  path.arcTo(QRectF{x1, y1 - (w / 2), w, w}, 0, 180);
  painter.setPen(QPen{Qt::white, lampRadius / 2});
  painter.fillPath(path, Qt::black);
  painter.drawPath(path);

  painter.setPen(Qt::NoPen);

  painter.setBrush(signalRed);
  painter.drawEllipse(QPointF{r.center().x(), r.center().y() - 2 * lampRadius}, lampRadius, lampRadius);

  painter.setBrush(signalYellow);
  painter.drawEllipse(r.center(), lampRadius, lampRadius);

  painter.setBrush(signalGreen);
  painter.drawEllipse(QPointF{r.center().x(), r.center().y() + 2 * lampRadius}, lampRadius, lampRadius);

  painter.restore();
}
