/**
 * client/src/board/tilepainter.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2021 Reinder Feenstra
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

#include "tilepainter.hpp"
#include <cmath>
#include <QtMath>
#include <QPainterPath>
#include "../utils/rectf.hpp"

TilePainter::TilePainter(QPainter& painter, int tileSize) :
  m_trackWidth{tileSize / 5},
  m_turnoutMargin{tileSize / 10},
  m_trackPen(trackColor, m_trackWidth, Qt::SolidLine, Qt::FlatCap),
  m_turnoutStatePen(Qt::blue, (m_trackWidth + 1) / 2, Qt::SolidLine, Qt::FlatCap),
  m_painter{painter}
{
}

void TilePainter::draw(TileId id, const QRectF& r, TileRotate rotate)
{
  switch(id)
  {
    case TileId::RailStraight:
      setTrackPen();
      drawStraight(r, rotate);
      break;

    case TileId::RailCurve45:
      setTrackPen();
      drawCurve45(r, rotate);
      break;

    case TileId::RailCurve90:
      setTrackPen();
      drawCurve90(r, rotate);
      break;

    case TileId::RailCross45:
      setTrackPen();
      drawStraight(r, rotate);
      drawStraight(r, rotate - TileRotate::Deg45);
      break;

    case TileId::RailCross90:
      setTrackPen();
      drawStraight(r, rotate);
      drawStraight(r, rotate + TileRotate::Deg90);
      break;

    case TileId::RailTurnoutLeft45:
    case TileId::RailTurnoutLeft90:
    case TileId::RailTurnoutLeftCurved:
    case TileId::RailTurnoutRight45:
    case TileId::RailTurnoutRight90:
    case TileId::RailTurnoutRightCurved:
    case TileId::RailTurnoutWye:
    case TileId::RailTurnout3Way:
    case TileId::RailTurnoutSingleSlip:
    case TileId::RailTurnoutDoubleSlip:
      drawTurnout(id, r, rotate);
      break;

    case TileId::RailSensor:
      drawSensor(id, r, rotate);
      break;

    case TileId::RailBufferStop:
      setTrackPen();
      drawBufferStop(r, rotate);
      break;

    case TileId::RailSignal2Aspect:
    case TileId::RailSignal3Aspect:
      drawSignal(id, r, rotate);
      break;

    case TileId::RailBlock:
      drawBlock(id, r, rotate);
      break;

    case TileId::None:
    case TileId::ReservedForFutureExpension:
      break;
  }
}

void TilePainter::drawSensor(TileId id, const QRectF& r, TileRotate rotate, TriState state)
{
  switch(id)
  {
    case TileId::RailSensor:
    {
      setTrackPen();
      drawStraight(r, rotate);
      const qreal sz = r.width() / 4;
      drawLED(r.adjusted(sz, sz, -sz, -sz), sensorStateToColor(state), trackColor);
      break;
    }
    default:
      break;
  }
}

void TilePainter::drawTurnout(TileId id, const QRectF& r, TileRotate rotate, TurnoutPosition position)
{
  switch(id)
  {
    case TileId::RailTurnoutLeft45:
      setTrackPen();
      drawStraight(r, rotate);
      drawCurve45(r, rotate);

      setTurnoutStatePen();
      switch(position)
      {
        case TurnoutPosition::Straight:
          drawStraight(turnoutStateRect(r), rotate);
          break;

        case TurnoutPosition::Left:
          drawCurve45(turnoutStateRect(r), rotate);
          break;

        default:
          break;
      }
      break;

    case TileId::RailTurnoutLeft90:
      setTrackPen();
      drawStraight(r, rotate);
      drawCurve90(r, rotate);

      setTurnoutStatePen();
      switch(position)
      {
        case TurnoutPosition::Straight:
          drawStraight(turnoutStateRect(r), rotate);
          break;

        case TurnoutPosition::Left:
          drawCurve90(turnoutStateRect(r), rotate);
          break;

        default:
          break;
      }
      break;

    case TileId::RailTurnoutLeftCurved:
      setTrackPen();
      drawCurve45(r, rotate);
      drawCurve90(r, rotate);

      setTurnoutStatePen();
      switch(position)
      {
        case TurnoutPosition::Straight:
          drawCurve45(turnoutStateRect(r), rotate);
          break;

        case TurnoutPosition::Left:
          drawCurve90(turnoutStateRect(r), rotate);
          break;

        default:
          break;
      }
      break;

    case TileId::RailTurnoutRight45:
      setTrackPen();
      drawStraight(r, rotate);
      drawCurve45(r, rotate + TileRotate::Deg225);

      setTurnoutStatePen();
      switch(position)
      {
        case TurnoutPosition::Straight:
          drawStraight(turnoutStateRect(r), rotate);
          break;

        case TurnoutPosition::Right:
          drawCurve45(turnoutStateRect(r), rotate + TileRotate::Deg225);
          break;

        default:
          break;
      }
      break;

    case TileId::RailTurnoutRight90:
      setTrackPen();
      drawStraight(r, rotate);
      drawCurve90(r, rotate + TileRotate::Deg270);

      setTurnoutStatePen();
      switch(position)
      {
        case TurnoutPosition::Straight:
          drawStraight(turnoutStateRect(r), rotate);
          break;

        case TurnoutPosition::Right:
          drawCurve90(turnoutStateRect(r), rotate + TileRotate::Deg270);
          break;

        default:
          break;
      }
      break;

    case TileId::RailTurnoutRightCurved:
      setTrackPen();
      drawCurve45(r, rotate + TileRotate::Deg225);
      drawCurve90(r, rotate + TileRotate::Deg270);

      setTurnoutStatePen();
      switch(position)
      {
        case TurnoutPosition::Straight:
          drawCurve45(turnoutStateRect(r), rotate + TileRotate::Deg225);
          break;

        case TurnoutPosition::Right:
          drawCurve90(turnoutStateRect(r), rotate + TileRotate::Deg270);
          break;

        default:
          break;
      }
      break;

    case TileId::RailTurnoutWye:
      setTrackPen();
      drawCurve45(r, rotate);
      drawCurve45(r, rotate + TileRotate::Deg225);

      setTurnoutStatePen();
      switch(position)
      {
        case TurnoutPosition::Left:
          drawCurve45(turnoutStateRect(r), rotate);
          break;

        case TurnoutPosition::Right:
          drawCurve45(turnoutStateRect(r), rotate + TileRotate::Deg225);
          break;

        default:
          break;
      }
      break;

    case TileId::RailTurnout3Way:
      setTrackPen();
      drawStraight(r, rotate);
      drawCurve45(r, rotate);
      drawCurve45(r, rotate + TileRotate::Deg225);

      setTurnoutStatePen();
      switch(position)
      {
        case TurnoutPosition::Straight:
          drawStraight(turnoutStateRect(r), rotate);
          break;

        case TurnoutPosition::Left:
          drawCurve45(turnoutStateRect(r), rotate);
          break;

        case TurnoutPosition::Right:
          drawCurve45(turnoutStateRect(r), rotate + TileRotate::Deg225);
          break;

        default:
          break;
      }
      break;

    case TileId::RailTurnoutSingleSlip:
      setTrackPen();
      drawStraight(r, rotate);
      drawStraight(r, rotate - TileRotate::Deg45);
      drawCurve45(r, rotate);

      setTurnoutStatePen();
      switch(position)
      {
        case TurnoutPosition::Crossed:
          drawStraight(turnoutStateRect(r), rotate);
          drawStraight(turnoutStateRect(r), rotate - TileRotate::Deg45);
          break;

        case TurnoutPosition::Diverged:
          drawCurve45(turnoutStateRect(r), rotate);
          break;

        default:
          break;
      }
      break;

    case TileId::RailTurnoutDoubleSlip:
      setTrackPen();
      drawStraight(r, rotate);
      drawStraight(r, rotate - TileRotate::Deg45);
      drawCurve45(r, rotate);
      drawCurve45(r, rotate + TileRotate::Deg180);

      setTurnoutStatePen();
      switch(position)
      {
        case TurnoutPosition::Crossed:
          drawStraight(turnoutStateRect(r), rotate);
          drawStraight(turnoutStateRect(r), rotate - TileRotate::Deg45);
          break;

        case TurnoutPosition::Diverged:
          drawCurve45(turnoutStateRect(r), rotate);
          drawCurve45(turnoutStateRect(r), rotate + TileRotate::Deg180);
          break;

        default:
          break;
      }
      break;

    default:
      assert(false);
      break;
  }
}

void TilePainter::drawSignal(TileId id, const QRectF& r, TileRotate rotate, SignalAspect aspect)
{
  switch(id)
  {
    case TileId::RailSignal2Aspect:
      setTrackPen();
      drawStraight(r, rotate);
      drawSignal2Aspect(r, rotate, aspect);
      break;

    case TileId::RailSignal3Aspect:
      setTrackPen();
      drawStraight(r, rotate);
      drawSignal3Aspect(r, rotate, aspect);
      break;

    default:
      assert(false);
      break;
  }
}

void TilePainter::drawBlock(TileId id, const QRectF& r, TileRotate rotate, BlockState state, const std::vector<BlockState> subStates)
{
  switch(id)
  {
    case TileId::RailBlock:
      drawRailBlock(r, rotate, state, subStates);
      break;

    default:
      assert(false);
      break;
  }
}

//=============================================================================

void TilePainter::setBlockStateBrush(BlockState value)
{
  switch(value)
  {
    case BlockState::Occupied:
      m_painter.setBrush(blockBrushOccupied);
      break;

    case BlockState::Free:
      m_painter.setBrush(blockBrushFree);
      break;

    case BlockState::Unknown:
      m_painter.setBrush(blockBrushUnknown);
      break;
  }
}

void TilePainter::drawStraight(const QRectF& r, TileRotate rotate)
{
  switch(rotate)
  {
    case TileRotate::Deg0:
    case TileRotate::Deg180:
      m_painter.drawLine(bottomCenter(r), topCenter(r));
      break;

    case TileRotate::Deg45:
    case TileRotate::Deg225:
      m_painter.drawLine(r.bottomLeft(), r.topRight());
      break;

    case TileRotate::Deg90:
    case TileRotate::Deg270:
      m_painter.drawLine(centerLeft(r), centerRight(r));
      break;

    case TileRotate::Deg135:
    case TileRotate::Deg315:
      m_painter.drawLine(r.bottomRight(), r.topLeft());
      break;
  }
}

void TilePainter::drawCurve45(const QRectF& r, TileRotate rotate)
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
  m_painter.drawPath(path);
}

void TilePainter::drawCurve90(QRectF r, TileRotate rotate)
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
      m_painter.drawArc(r, 0 * 16, spanAngle);
      break;

    case TileRotate::Deg45:
      r.translate(-size, 0);
      m_painter.drawArc(r, 315 * 16, spanAngle);
      break;

    case TileRotate::Deg90:
      r.translate(-size * 0.5, -size * 0.5);
      m_painter.drawArc(r, 270 * 16, spanAngle);
      break;

    case TileRotate::Deg135:
      r.translate(0, -size);
      m_painter.drawArc(r, 225 * 16, spanAngle);
      break;

    case TileRotate::Deg180:
      r.translate(size * 0.5, -size * 0.5);
      m_painter.drawArc(r, 180 * 16, spanAngle);
      break;

    case TileRotate::Deg225:
      r.translate(size, 0);
      m_painter.drawArc(r, 135 * 16, spanAngle);
      break;

    case TileRotate::Deg270:
      r.translate(size * 0.5, size * 0.5);
      m_painter.drawArc(r, 90 * 16, spanAngle);
      break;

    case TileRotate::Deg315:
      r.translate(0, size);
      m_painter.drawArc(r, 45 * 16, spanAngle);
      break;
  }
}

void TilePainter::drawBufferStop(const QRectF& r, TileRotate rotate)
{
  const qreal stopSize = 0.5 + qRound(r.width() / 4);
  const qreal stopSizeDiagonal = stopSize / std::sqrt(2);
  const qreal cx = r.center().x();
  const qreal cy = r.center().y();

  switch(rotate)
  {
    case TileRotate::Deg0:
      m_painter.drawLine(bottomCenter(r), r.center());
      m_painter.drawLine(QPointF{cx - stopSize, cy}, QPointF{cx + stopSize, cy});
      break;

    case TileRotate::Deg45:
      m_painter.drawLine(r.bottomLeft(), r.center());
      m_painter.drawLine(QPointF{cx - stopSizeDiagonal, cy - stopSizeDiagonal}, QPointF{cx + stopSizeDiagonal, cy + stopSizeDiagonal});
      break;

    case TileRotate::Deg90:
      m_painter.drawLine(centerLeft(r), r.center());
      m_painter.drawLine(QPointF{cx, cy - stopSize}, QPointF{cx, cy + stopSize});
      break;

    case TileRotate::Deg135:
      m_painter.drawLine(r.topLeft(), r.center());
      m_painter.drawLine(QPointF{cx - stopSizeDiagonal, cy + stopSizeDiagonal}, QPointF{cx + stopSizeDiagonal, cy - stopSizeDiagonal});
      break;

    case TileRotate::Deg180:
      m_painter.drawLine(topCenter(r), r.center());
      m_painter.drawLine(QPointF{cx - stopSize, cy}, QPointF{cx + stopSize, cy});
      break;

    case TileRotate::Deg225:
      m_painter.drawLine(r.topRight(), r.center());
      m_painter.drawLine(QPointF{cx - stopSizeDiagonal, cy - stopSizeDiagonal}, QPointF{cx + stopSizeDiagonal, cy + stopSizeDiagonal});
      break;

    case TileRotate::Deg270:
      m_painter.drawLine(centerRight(r), r.center());
      m_painter.drawLine(QPointF{cx, cy - stopSize}, QPointF{cx, cy + stopSize});
      break;

    case TileRotate::Deg315:
      m_painter.drawLine(r.bottomRight(), r.center());
      m_painter.drawLine(QPointF{cx - stopSizeDiagonal, cy + stopSizeDiagonal}, QPointF{cx + stopSizeDiagonal, cy - stopSizeDiagonal});
      break;
  }
}

void TilePainter::drawSignal2Aspect(QRectF r, TileRotate rotate, SignalAspect aspect)
{
  m_painter.save();
  m_painter.translate(r.center());
  m_painter.rotate(toDeg(rotate));
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
  m_painter.setPen(QPen{Qt::white, lampRadius / 2});
  m_painter.fillPath(path, Qt::black);
  m_painter.drawPath(path);

  m_painter.setPen(Qt::NoPen);
  switch(aspect)
  {
    case SignalAspect::Stop:
      m_painter.setBrush(signalRed);
      m_painter.drawEllipse(QPointF{r.center().x(), r.center().y() - lampRadius}, lampRadius, lampRadius);
      break;

    case SignalAspect::Proceed:
      m_painter.setBrush(signalGreen);
      m_painter.drawEllipse(QPointF{r.center().x(), r.center().y() + lampRadius}, lampRadius, lampRadius);
      break;

    default:
      break;
  }

  m_painter.restore();
}

void TilePainter::drawLED(const QRectF& r, const QColor& color, const QColor& borderColor)
{
  if(borderColor.isValid())
    m_painter.setPen(borderColor);
  else
    m_painter.setPen(Qt::NoPen);

  m_painter.setBrush(color);

  m_painter.drawEllipse(r);
}

void TilePainter::drawSignal3Aspect(QRectF r, TileRotate rotate, SignalAspect aspect)
{
  m_painter.save();
  m_painter.translate(r.center());
  m_painter.rotate(toDeg(rotate));
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
  m_painter.setPen(QPen{Qt::white, lampRadius / 2});
  m_painter.fillPath(path, Qt::black);
  m_painter.drawPath(path);

  m_painter.setPen(Qt::NoPen);
  switch(aspect)
  {
    case SignalAspect::Stop:
      m_painter.setBrush(signalRed);
      m_painter.drawEllipse(QPointF{r.center().x(), r.center().y() - 2 * lampRadius}, lampRadius, lampRadius);
      break;

    case SignalAspect::ProceedReducedSpeed:
      m_painter.setBrush(signalYellow);
      m_painter.drawEllipse(r.center(), lampRadius, lampRadius);
      break;

    case SignalAspect::Proceed:
      m_painter.setBrush(signalGreen);
      m_painter.drawEllipse(QPointF{r.center().x(), r.center().y() + 2 * lampRadius}, lampRadius, lampRadius);
      break;

    default:
      break;
  }

  m_painter.restore();
}

void TilePainter::drawRailBlock(const QRectF& r, TileRotate rotate, BlockState state, const std::vector<BlockState> subStates)
{
  setTrackPen();

  if(rotate == TileRotate::Deg0)
  {
    m_painter.drawLine(topCenter(r), bottomCenter(r));
    setBlockStateBrush(state);
    m_painter.setPen(blockPen);
    const qreal m = 0.5 + qFloor(r.width() / 10);
    const QRectF block = r.adjusted(m, m, -m, -m);
    m_painter.drawRect(block);

    if(!subStates.empty())
    {
      const qreal height = block.height() / subStates.size();
      const qreal width = qRound(block.width() / 5);
      qreal top = block.top();
      for(BlockState subState : subStates)
      {
        setBlockStateBrush(subState);
        m_painter.drawRect(QRectF(block.left(), qRound(top) - 0.5, width, qRound(top + height) - qRound(top)));
        top += height;
      }
    }
  }
  else if(rotate == TileRotate::Deg90)
  {
    m_painter.drawLine(centerLeft(r), centerRight(r));
    setBlockStateBrush(state);
    m_painter.setPen(blockPen);
    const qreal m = 0.5 + qFloor(r.height() / 10);
    const QRectF block = r.adjusted(m, m, -m, -m);
    m_painter.drawRect(block);

    if(!subStates.empty())
    {
      const qreal width = block.width() / subStates.size();
      const qreal height = qRound(block.height() / 5);
      const qreal top = block.bottom() - height;
      double left = block.left();
      for(BlockState subState : subStates)
      {
        setBlockStateBrush(subState);
        m_painter.drawRect(QRectF(qRound(left) - 0.5, top, qRound(left + width) - qRound(left), height));
        left += width;
      }
    }
  }
  else
    assert(false);
}
