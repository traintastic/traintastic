/**
 * client/src/board/tilepainter.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2025 Reinder Feenstra
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
#include <traintastic/enum/blocktraindirection.hpp>
#include "boardcolorscheme.hpp"
#include "../network/object.tpp"
#include "../network/object/blockrailtile.hpp"
#include "../network/object/trainblockstatus.hpp"
#include "../network/abstractvectorproperty.hpp"
#include "../settings/boardsettings.hpp"
#include "../utils/rectf.hpp"

TilePainter::TilePainter(QPainter& painter, int tileSize, const BoardColorScheme& colorScheme) :
  m_colorScheme{colorScheme},
  m_showBlockSensorStates{BoardSettings::instance().showBlockSensorStates},
  m_turnoutDrawState{BoardSettings::instance().turnoutDrawState},
  m_trackWidth{tileSize / 5},
  m_turnoutMargin{tileSize / 10},
  m_blockPen{m_colorScheme.track},
  m_trackPen(m_colorScheme.track, m_trackWidth, Qt::SolidLine, Qt::FlatCap),
  m_trackDisabledPen(m_colorScheme.trackDisabled, m_trackWidth, Qt::SolidLine, Qt::FlatCap),
  m_trackReservedPen(m_colorScheme.trackReserved, m_trackWidth, Qt::SolidLine, Qt::FlatCap),
  m_trackReservedDisabledPen(m_colorScheme.trackReservedDisabled, m_trackWidth, Qt::SolidLine, Qt::FlatCap),
  m_trackErasePen(m_colorScheme.background, m_trackWidth * 2, Qt::SolidLine, Qt::FlatCap),
  m_turnoutStatePen(m_colorScheme.turnoutState, (m_trackWidth + 1) / 2, Qt::SolidLine, Qt::FlatCap),
  m_painter{painter}
{
}

void TilePainter::draw(TileId id, const QRectF& r, TileRotate rotate, bool isReserved)
{
  switch(id)
  {
    case TileId::RailStraight:
      setTrackPen(isReserved);
      drawStraight(r, rotate);
      break;

    case TileId::RailCurve45:
      setTrackPen(isReserved);
      drawCurve45(r, rotate);
      break;

    case TileId::RailCurve90:
      setTrackPen(isReserved);
      drawCurve90(r, rotate);
      break;

    case TileId::RailCross45:
    case TileId::RailCross90:
      drawCross(id, r, rotate);
      break;

    case TileId::RailBridge45Left:
    case TileId::RailBridge45Right:
    case TileId::RailBridge90:
      drawBridge(id, r, rotate);
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
      setTrackPen(isReserved);
      drawBufferStop(r, rotate);
      break;

    case TileId::RailSignal2Aspect:
    case TileId::RailSignal3Aspect:
      drawSignal(id, r, rotate, isReserved);
      break;

    case TileId::RailBlock:
      drawBlock(id, r, rotate);
      break;

    case TileId::RailTunnel:
    {
      setTrackPen(isReserved);
      drawStraight(r, rotate);

      // tunnel arc:
      const int angle = -toDeg(rotate) * 16 - 45 * 8; // - 22.5 deg
      const int angleLength = 225 * 16; // 225 deg
      const qreal m = r.width() / 5;
      const QRectF rArc = r.adjusted(m, m, -m, -m);

      m_painter.setPen(QPen(m_colorScheme.background, m_trackWidth, Qt::SolidLine, Qt::FlatCap));
      m_painter.drawArc(rArc, angle, angleLength);
      m_painter.setPen(QPen(m_colorScheme.track, m_trackWidth / 2., Qt::SolidLine, Qt::FlatCap));
      m_painter.drawArc(rArc, angle, angleLength);
      break;
    }
    case TileId::RailDirectionControl:
      drawDirectionControl(id, r, rotate, isReserved);
      break;

    case TileId::RailOneWay:
    {
      setTrackPen(isReserved);
      drawStraight(r, rotate);
      m_painter.setBrush(m_painter.pen().color());

      const qreal m = r.width() / 4;
      QRectF rTriangle = r.adjusted(m, m, -m, -m);
      m_painter.save();
      m_painter.translate(rTriangle.center());
      m_painter.rotate(toDeg(rotate));
      rTriangle.moveCenter(QPointF{0, 0});
      QPen pen = m_painter.pen();
      pen.setJoinStyle(Qt::RoundJoin);
      m_painter.setPen(pen);
      drawTriangle(rTriangle);
      m_painter.restore();
      break;
    }
    case TileId::PushButton:
      drawPushButton(r);
      break;

    case TileId::RailLink:
      setTrackPen(isReserved);
      drawLink(r, rotate);
      break;

    case TileId::RailDecoupler:
      drawRailDecoupler(r, rotate, isReserved);
      break;

    case TileId::RailNXButton:
      drawRailNX(r, rotate, isReserved);
      break;

    case TileId::Label:
      drawLabel(r, rotate);
      break;

    case TileId::Switch:
      drawSwitch(r);
      break;

    case TileId::None:
    case TileId::HiddenRailCrossOver:
    case TileId::ReservedForFutureExpension:
      break;
  }
}

void TilePainter::drawBridge(TileId id, const QRectF& r, TileRotate rotate, bool isReservedAC, bool isReservedBD)
{
  switch(id)
  {
    case TileId::RailBridge45Left:
      setTrackPen(isReservedAC);
      drawStraight(r, rotate);
      setTrackErasePen();
      drawStraight(r, rotate - TileRotate::Deg45);
      setTrackPen(isReservedBD);
      drawStraight(r, rotate - TileRotate::Deg45);
      break;

    case TileId::RailBridge45Right:
      setTrackPen(isReservedAC);
      drawStraight(r, rotate);
      setTrackErasePen();
      drawStraight(r, rotate + TileRotate::Deg45);
      setTrackPen(isReservedBD);
      drawStraight(r, rotate + TileRotate::Deg45);
      break;

    case TileId::RailBridge90:
      setTrackPen(isReservedAC);
      drawStraight(r, rotate);
      setTrackErasePen();
      drawStraight(r, rotate + TileRotate::Deg90);
      setTrackPen(isReservedBD);
      drawStraight(r, rotate + TileRotate::Deg90);
      break;

    default:
      break;
  }
}

void TilePainter::drawCross(TileId id, const QRectF& r, TileRotate rotate, CrossState reservedState)
{
  switch(id)
  {
    case TileId::RailCross45:
      setTrackPen();
      if(reservedState != CrossState::AC)
      {
        drawStraight(r, rotate);
      }
      if(reservedState != CrossState::BD)
      {
        drawStraight(r, rotate - TileRotate::Deg45);
      }

      if(reservedState != CrossState::Unset)
      {
        setTrackPen(true);
        switch(reservedState)
        {
          case CrossState::AC:
            drawStraight(r, rotate);
            break;

          case CrossState::BD:
            drawStraight(r, rotate - TileRotate::Deg45);
            break;

          default:
            break;
        }
      }
      break;

    case TileId::RailCross90:
      setTrackPen();
      if(reservedState != CrossState::AC)
      {
        drawStraight(r, rotate);
      }
      if(reservedState != CrossState::BD)
      {
        drawStraight(r, rotate - TileRotate::Deg90);
      }

      if(reservedState != CrossState::Unset)
      {
        setTrackPen(true);
        switch(reservedState)
        {
          case CrossState::AC:
            drawStraight(r, rotate);
            break;

          case CrossState::BD:
            drawStraight(r, rotate - TileRotate::Deg90);
            break;

          default:
            break;
        }
      }
      break;

    default:
      break;
  }
}

void TilePainter::drawSensor(TileId id, const QRectF& r, TileRotate rotate, bool isReserved, SensorState state)
{
  switch(id)
  {
    case TileId::RailSensor:
    {
      setTrackPen(isReserved);
      drawStraight(r, rotate);
      const qreal sz = r.width() / 4;
      drawLED(r.adjusted(sz, sz, -sz, -sz), sensorStateToColor(state), m_colorScheme.track);
      break;
    }
    default:
      break;
  }
}

void TilePainter::drawDirectionControl(TileId id, const QRectF& r, TileRotate rotate, bool isReserved, DirectionControlState state)
{
  switch(id)
  {
    case TileId::RailDirectionControl:
    {
      setTrackPen(isReserved);
      drawStraight(r, rotate);

      QPen pen{m_trackPen};
      const qreal w = pen.width() / 2;
      pen.setWidth(w);
      switch(state)
      {
        case DirectionControlState::None:
          pen.setColor(Qt::red);
          break;

        case DirectionControlState::AtoB:
        case DirectionControlState::BtoA:
          pen.setColor(Qt::blue);
          break;


        case DirectionControlState::Both:
          pen.setColor(Qt::darkGreen);
          break;
      }
      m_painter.setPen(pen);
      m_painter.setBrush(pen.color());
      m_painter.drawEllipse(r.adjusted(w, w, -w, -w));

      const qreal m = r.width() / 3;
      QRectF rSign = r.adjusted(m, m, -m, -m);
      m_painter.save();
      m_painter.translate(rSign.center());
      m_painter.rotate(toDeg(rotate + (state == DirectionControlState::BtoA ? TileRotate::Deg180 : TileRotate::Deg0)));
      rSign.moveCenter(QPointF{0, 0});
      pen = m_trackPen;
      pen.setColor(Qt::white);
      pen.setBrush(pen.color());
      pen.setCapStyle(Qt::RoundCap);
      pen.setJoinStyle(Qt::RoundJoin);
      m_painter.setPen(pen);

      switch(state)
      {
        case DirectionControlState::None:
          m_painter.drawLine(centerLeft(rSign), centerRight(rSign));
          break;

        case DirectionControlState::AtoB:
        case DirectionControlState::BtoA:
          drawTriangle(rSign);
          break;

        case DirectionControlState::Both:
          m_painter.drawLine(topCenter(rSign), bottomCenter(rSign));
          break;
      }

      m_painter.restore();
      break;
    }
    default:
      break;
  }
}

void TilePainter::drawTurnout(TileId id, const QRectF& r, TileRotate rotate, TurnoutPosition reservedPosition, TurnoutPosition position)
{
  switch(id)
  {
    case TileId::RailTurnoutLeft45:
    case TileId::RailTurnoutLeft90:
    case TileId::RailTurnoutLeftCurved:
    case TileId::RailTurnoutRight45:
    case TileId::RailTurnoutRight90:
    case TileId::RailTurnoutRightCurved:
    case TileId::RailTurnoutWye:
    case TileId::RailTurnout3Way:
      drawTurnoutStandard(id, r, rotate, reservedPosition, position);
      break;

    case TileId::RailTurnoutSingleSlip:
    case TileId::RailTurnoutDoubleSlip:
      drawTurnoutSlip(id, r, rotate, reservedPosition, position);
      break;

    default:
      assert(false);
      break;
  }
}

void TilePainter::drawSignal(TileId id, const QRectF& r, TileRotate rotate, bool isReserved, SignalAspect aspect)
{
  switch(id)
  {
    case TileId::RailSignal2Aspect:
      setTrackPen(isReserved);
      drawStraight(r, rotate);
      drawSignalDirection(r, rotate);
      drawSignal2Aspect(r, rotate, aspect);
      break;

    case TileId::RailSignal3Aspect:
      setTrackPen(isReserved);
      drawStraight(r, rotate);
      drawSignalDirection(r, rotate);
      drawSignal3Aspect(r, rotate, aspect);
      break;

    default:
      assert(false);
      break;
  }
}

void TilePainter::drawBlock(TileId id, const QRectF& r, TileRotate rotate, bool isReservedA, bool isReservedB, const ObjectPtr& blockTile)
{
  switch(id)
  {
    case TileId::RailBlock:
      drawRailBlock(r, rotate, isReservedA, isReservedB, blockTile);
      break;

    default:
      assert(false);
      break;
  }
}

void TilePainter::drawPushButton(const QRectF& r, Color color, Color textColor, const QString& text)
{
  const auto size = std::min(r.height(), r.width());
  m_painter.setPen(QPen(Qt::gray, size / 10));
  m_painter.setBrush(toQColor(color));
  const qreal radius = size * 0.4;
  if(r.height() == r.width())
  {
    m_painter.drawEllipse(r.center(), radius, radius);
  }
  else
  {
    const auto margin = size * 0.1;
    m_painter.drawRoundedRect(r.adjusted(margin, margin, -margin, -margin), radius, radius);
  }

  if(!text.isEmpty())
  {
    const auto margin = size * 0.2;
    m_painter.setPen(toQColor(textColor));
    m_painter.drawText(r.adjusted(margin, margin, -margin, -margin), text, QTextOption(Qt::AlignCenter));
  }
}

void TilePainter::drawSwitch(const QRectF& r, bool value, Color colorOn, Color colorOff)
{
  m_painter.setPen(QPen(Qt::gray, r.width() / 10));
  m_painter.setBrush(toQColor(value ? colorOn : colorOff));
  const qreal margin = r.width() * 0.1;
  const qreal radius = r.width() * 0.15;
  m_painter.drawRoundedRect(r.adjusted(margin, margin, -margin, -margin), radius, radius);
}

//=============================================================================

QColor TilePainter::sensorStateToColor(SensorState value) const
{
  switch(value)
  {
    case SensorState::Occupied:
      return m_colorScheme.sensorOccupied;

    case SensorState::Free:
      return m_colorScheme.sensorFree;

    case SensorState::Idle:
      return m_colorScheme.sensorIdle;

    case SensorState::Triggered:
      return m_colorScheme.sensorTriggered;

    case SensorState::Unknown:
      return m_colorScheme.sensorUnknown;
  }
  assert(false);
  return QColor();
}

void TilePainter::setBlockStateBrush(BlockState value)
{
  switch(value)
  {
    case BlockState::Occupied:
      m_painter.setBrush(m_colorScheme.blockOccupied);
      break;

    case BlockState::Free:
      m_painter.setBrush(m_colorScheme.blockFree);
      break;

    case BlockState::Reserved:
      m_painter.setBrush(m_colorScheme.blockReserved);
      break;

    case BlockState::Unknown:
      m_painter.setBrush(m_colorScheme.blockUnknown);
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

void TilePainter::drawLink(const QRectF& r, TileRotate rotate)
{
  switch(rotate)
  {
    case TileRotate::Deg0:
      m_painter.drawLine(bottomCenter(r), r.center());
      break;

    case TileRotate::Deg45:
      m_painter.drawLine(r.bottomLeft(), r.center());
      break;

    case TileRotate::Deg90:
      m_painter.drawLine(centerLeft(r), r.center());
      break;

    case TileRotate::Deg135:
      m_painter.drawLine(r.topLeft(), r.center());
      break;

    case TileRotate::Deg180:
      m_painter.drawLine(topCenter(r), r.center());
      break;

    case TileRotate::Deg225:
      m_painter.drawLine(r.topRight(), r.center());
      break;

    case TileRotate::Deg270:
      m_painter.drawLine(centerRight(r), r.center());
      break;

    case TileRotate::Deg315:
      m_painter.drawLine(r.bottomRight(), r.center());
      break;
  }

  const qreal radius = r.width() / 6;
  m_painter.setBrush(m_painter.pen().color());
  m_painter.drawEllipse(r.center(), radius, radius);
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

void TilePainter::drawTriangle(const QRectF& r)
{
  const std::array<QPointF, 3> points = {{
    {r.center().x(), r.top()},
    {r.right(), r.bottom()},
    {r.left(), r.bottom()}}};

  m_painter.drawConvexPolygon(points.data(), static_cast<int>(points.size()));
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

void TilePainter::drawTurnoutStandard(TileId id, const QRectF& r, TileRotate rotate, TurnoutPosition reservedPosition, TurnoutPosition position)
{
  const bool hasLeft90 =
    id == TileId::RailTurnoutLeft90 ||
    id == TileId::RailTurnoutLeftCurved;

  const bool hasLeft =
    hasLeft90 ||
    id == TileId::RailTurnoutLeft45 ||
    id == TileId::RailTurnoutWye ||
    id == TileId::RailTurnout3Way;

  const bool hasStraight =
    id == TileId::RailTurnoutLeft45 ||
    id == TileId::RailTurnoutLeft90 ||
    id == TileId::RailTurnoutLeftCurved ||
    id == TileId::RailTurnoutRight45 ||
    id == TileId::RailTurnoutRight90 ||
    id == TileId::RailTurnoutRightCurved ||
    id == TileId::RailTurnout3Way;

  const bool hasRight90 =
    id == TileId::RailTurnoutRight90 ||
    id == TileId::RailTurnoutRightCurved;

  const bool hasRight =
    hasRight90 ||
    id == TileId::RailTurnoutRight45 ||
    id == TileId::RailTurnoutWye ||
    id == TileId::RailTurnout3Way;

  setTurnoutPen();
  if(hasLeft && (m_turnoutDrawState || position != TurnoutPosition::Left) && reservedPosition != TurnoutPosition::Left)
  {
    if(hasLeft90)
    {
      drawCurve90(r, rotate);
    }
    else
    {
      drawCurve45(r, rotate);
    }
  }
  if(hasStraight && (m_turnoutDrawState || position != TurnoutPosition::Straight) && reservedPosition != TurnoutPosition::Straight)
  {
    if(id == TileId::RailTurnoutLeftCurved)
    {
      drawCurve45(r, rotate);
    }
    else if(id == TileId::RailTurnoutRightCurved)
    {
      drawCurve45(r, rotate + TileRotate::Deg225);
    }
    else
    {
      drawStraight(r, rotate);
    }
  }
  if(hasRight && (m_turnoutDrawState || position != TurnoutPosition::Right) && reservedPosition != TurnoutPosition::Right)
  {
    if(hasRight90)
    {
      drawCurve90(r, rotate + TileRotate::Deg270);
    }
    else
    {
      drawCurve45(r, rotate + TileRotate::Deg225);
    }
  }

  if(reservedPosition != TurnoutPosition::Unknown)
  {
    m_painter.setPen(position == reservedPosition ? m_trackReservedPen : m_trackReservedDisabledPen);

    switch(reservedPosition)
    {
      case TurnoutPosition::Left:
        if(hasLeft90)
        {
          drawCurve90(r, rotate);
        }
        else
        {
          drawCurve45(r, rotate);
        }
        break;

      case TurnoutPosition::Straight:
        if(id == TileId::RailTurnoutLeftCurved)
        {
          drawCurve45(r, rotate);
        }
        else if(id == TileId::RailTurnoutRightCurved)
        {
          drawCurve45(r, rotate + TileRotate::Deg225);
        }
        else
        {
          drawStraight(r, rotate);
        }
        break;

      case TurnoutPosition::Right:
        if(hasRight90)
        {
          drawCurve90(r, rotate + TileRotate::Deg270);
        }
        else
        {
          drawCurve45(r, rotate + TileRotate::Deg225);
        }
        break;

      default:
        break;
    }
  }

  if(m_turnoutDrawState || position != reservedPosition)
  {
    setTurnoutStatePen();
    switch(position)
    {
      case TurnoutPosition::Left:
        if(hasLeft90)
        {
          drawCurve90(turnoutStateRect(r), rotate);
        }
        else
        {
          drawCurve45(turnoutStateRect(r), rotate);
        }
        break;

      case TurnoutPosition::Straight:
        if(id == TileId::RailTurnoutLeftCurved)
        {
          drawCurve45(turnoutStateRect(r), rotate);
        }
        else if(id == TileId::RailTurnoutRightCurved)
        {
          drawCurve45(turnoutStateRect(r), rotate + TileRotate::Deg225);
        }
        else
        {
          drawStraight(turnoutStateRect(r), rotate);
        }
        break;

      case TurnoutPosition::Right:
        if(hasRight90)
        {
          drawCurve90(turnoutStateRect(r), rotate + TileRotate::Deg270);
        }
        else
        {
          drawCurve45(turnoutStateRect(r), rotate + TileRotate::Deg225);
        }
        break;

      default:
        break;
    }
  }
}

void TilePainter::drawTurnoutSlip(TileId id, const QRectF& r, TileRotate rotate, TurnoutPosition reservedPosition, TurnoutPosition position)
{
  //  Double:      Single:
  //      C            C
  //      |\           |
  //  B --+-- D    B --+-- D
  //     \|           \|
  //      A            A

  const bool isDoubleSlip = id == TileId::RailTurnoutDoubleSlip;

  const bool positionAB = position == TurnoutPosition::Left || position == TurnoutPosition::Diverged;
  const bool positionAC = position == TurnoutPosition::DoubleSlipStraightA || position == TurnoutPosition::Crossed;
  const bool positionBD = position == TurnoutPosition::DoubleSlipStraightB || position == TurnoutPosition::Crossed;
  const bool positionCD = isDoubleSlip && (position == TurnoutPosition::Right || position == TurnoutPosition::Diverged);

  const bool reservedPositionAB = reservedPosition == TurnoutPosition::Left || reservedPosition == TurnoutPosition::Diverged;
  const bool reservedPositionAC = reservedPosition == TurnoutPosition::DoubleSlipStraightA || reservedPosition == TurnoutPosition::Crossed;
  const bool reservedPositionBD = reservedPosition == TurnoutPosition::DoubleSlipStraightB || reservedPosition == TurnoutPosition::Crossed;
  const bool reservedPositionCD = isDoubleSlip && (reservedPosition == TurnoutPosition::Right || reservedPosition == TurnoutPosition::Diverged);

  setTurnoutPen();
  if((m_turnoutDrawState || !positionAB) && !reservedPositionAB)
  {
    drawCurve45(r, rotate);
  }
  if((m_turnoutDrawState || !positionAC) && !reservedPositionAC)
  {
    drawStraight(r, rotate);
  }
  if((m_turnoutDrawState || !positionBD) && !reservedPositionBD)
  {
    drawStraight(r, rotate - TileRotate::Deg45);
  }
  if(isDoubleSlip && (m_turnoutDrawState || !positionCD) && !reservedPositionCD)
  {
    drawCurve45(r, rotate + TileRotate::Deg180);
  }

  if(!m_turnoutDrawState)
  {
    setTrackPen();
    if(position == TurnoutPosition::Crossed)
    {
      if(!reservedPositionAC)
      {
        drawStraight(r, rotate);
      }
      if(!reservedPositionBD)
      {
        drawStraight(r, rotate - TileRotate::Deg45);
      }
    }
    else if(position == TurnoutPosition::Diverged)
    {
      if(!reservedPositionAB)
      {
        drawCurve45(r, rotate);
      }
      if(isDoubleSlip && !reservedPositionCD)
      {
        drawCurve45(r, rotate + TileRotate::Deg180);
      }
    }
  }

  if(reservedPosition != TurnoutPosition::Unknown)
  {
    if(reservedPositionAB && !positionAB)
    {
      m_painter.setPen(m_trackReservedDisabledPen);
      drawCurve45(r, rotate);
    }
    if(reservedPositionAC && !positionAC)
    {
      m_painter.setPen(m_trackReservedDisabledPen);
      drawStraight(r, rotate);
    }
    if(reservedPositionBD && !positionBD)
    {
      m_painter.setPen(m_trackReservedDisabledPen);
      drawStraight(r, rotate - TileRotate::Deg45);
    }
    if(reservedPositionCD && !positionCD)
    {
      m_painter.setPen(m_trackReservedDisabledPen);
      drawCurve45(r, rotate + TileRotate::Deg180);
    }

    if(reservedPositionAB && positionAB)
    {
      m_painter.setPen(m_trackReservedPen);
      drawCurve45(r, rotate);
    }
    if(reservedPositionAC && positionAC)
    {
      m_painter.setPen(m_trackReservedPen);
      drawStraight(r, rotate);
    }
    if(reservedPositionBD && positionBD)
    {
      m_painter.setPen(m_trackReservedPen);
      drawStraight(r, rotate - TileRotate::Deg45);
    }
    if(reservedPositionCD && positionCD)
    {
      m_painter.setPen(m_trackReservedPen);
      drawCurve45(r, rotate + TileRotate::Deg180);
    }
  }

  setTurnoutStatePen();
  if(positionAB && (m_turnoutDrawState || reservedPositionAC || reservedPositionBD || (!reservedPositionAB && position != TurnoutPosition::Diverged)))
  {
    drawCurve45(turnoutStateRect(r), rotate);
  }
  if(positionAC && (m_turnoutDrawState || reservedPositionAB || reservedPositionCD || (!reservedPositionAC && position != TurnoutPosition::Crossed)))
  {
    drawStraight(turnoutStateRect(r), rotate);
  }
  if(positionBD && (m_turnoutDrawState || reservedPositionAB || reservedPositionCD || (!reservedPositionBD && position != TurnoutPosition::Crossed)))
  {
    drawStraight(turnoutStateRect(r), rotate - TileRotate::Deg45);
  }
  if(positionCD && (m_turnoutDrawState || reservedPositionAC || reservedPositionBD || (!reservedPositionCD && position != TurnoutPosition::Diverged)))
  {
    drawCurve45(turnoutStateRect(r), rotate + TileRotate::Deg180);
  }
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

void TilePainter::drawSignalDirection(QRectF r, TileRotate rotate)
{
  m_painter.save();
  m_painter.translate(r.center());
  m_painter.rotate(toDeg(rotate));
  r.moveCenter(QPointF{0, 0});

  QPainterPath path;
  qreal x;
  qreal y;
  if(isDiagonal(rotate))
  {
    x = r.width() * 0.55;
    y = r.height() * 0.075;
  }
  else
  {
    x = r.width() * 0.4;
    y = r.height() * 0.4;
  }
  path.moveTo(x, y);
  path.lineTo(x - r.width() * 0.15, y);
  path.lineTo(x - r.width() * 0.075, y - r.height() * 0.15);
  m_painter.fillPath(path, m_colorScheme.foreground);

  m_painter.restore();
}

void TilePainter::drawRailBlock(const QRectF& r, TileRotate rotate, bool isReservedA, bool isReservedB, const ObjectPtr& blockTile)
{
  const BlockState state = blockTile ? blockTile->getPropertyValueEnum<BlockState>("state", BlockState::Unknown) : BlockState::Unknown;
  std::vector<SensorState> subStates;
  QString label;

  if(blockTile)
  {
    if(m_showBlockSensorStates)
    {
      if(const auto* p = blockTile->getVectorProperty("sensor_states"))
      {
        const int size = p->size();
        subStates.resize(static_cast<size_t>(size));
        for(int i = 0; i < size; i++)
          subStates[i] = p->getEnum<SensorState>(i);
      }
    }

    if(auto* block = dynamic_cast<BlockRailTile*>(blockTile.get()))
    {
      if(block->trains().size() == 1)
      {
        if(auto* trainBlockStatus = dynamic_cast<TrainBlockStatus*>(block->trains()[0].get())) /*[[likely]]*/
        {
          if(const auto& train = trainBlockStatus->train())
          {
            if(trainBlockStatus->direction() == BlockTrainDirection::TowardsA)
              label += "< ";

            label += train->getPropertyValueString("name");

            if(trainBlockStatus->direction() == BlockTrainDirection::TowardsB)
              label +=  " >";
          }
          else if(auto identification = trainBlockStatus->identification(); !identification.isEmpty())
          {
            if(trainBlockStatus->direction() == BlockTrainDirection::TowardsA)
              label += "< ";

            label += identification;

            if(trainBlockStatus->direction() == BlockTrainDirection::TowardsB)
              label +=  " >";
          }
        }
      }
    }

    if(label.isEmpty())
      label = blockTile->getPropertyValueString("name");
  }

  if(rotate == TileRotate::Deg0)
  {
    setTrackPen(isReservedA);
    m_painter.drawLine(topCenter(r), r.center());
    setTrackPen(isReservedB);
    m_painter.drawLine(r.center(), bottomCenter(r));

    setBlockStateBrush(state);
    m_painter.setPen(m_blockPen);
    const qreal m = 0.5 + qFloor(r.width() / 10);
    QRectF block = r.adjusted(m, m, -m, -m);
    m_painter.drawRect(block);

    if(!subStates.empty())
    {
      const qreal height = block.height() / subStates.size();
      const qreal width = qRound(block.width() / 5);
      qreal top = block.top();
      for(SensorState subState : subStates)
      {
        m_painter.setBrush(sensorStateToColor(subState));
        m_painter.drawRect(QRectF(block.left(), qRound(top) - 0.5, width, qRound(top + height) - qRound(top)));
        top += height;
      }
      block.setLeft(block.left() + width);
    }

    if(!label.isEmpty())
    {
      m_painter.save();
      m_painter.translate(r.center());
      m_painter.rotate(90);
      m_painter.setPen(m_colorScheme.blockText);
      m_painter.drawText(QRectF{-block.height() / 2, -block.width() / 2, block.height(), block.width()}, label, QTextOption(Qt::AlignCenter));
      m_painter.restore();
    }
  }
  else if(rotate == TileRotate::Deg90)
  {
    setTrackPen(isReservedA);
    m_painter.drawLine(centerLeft(r), r.center());
    setTrackPen(isReservedB);
    m_painter.drawLine(r.center(), centerRight(r));

    setBlockStateBrush(state);
    m_painter.setPen(m_blockPen);
    const qreal m = 0.5 + qFloor(r.height() / 10);
    QRectF block = r.adjusted(m, m, -m, -m);
    m_painter.drawRect(block);

    if(!subStates.empty())
    {
      const qreal width = block.width() / subStates.size();
      const qreal height = qRound(block.height() / 5);
      const qreal top = block.bottom() - height;
      double left = block.left();
      for(const auto& subState : subStates)
      {
        m_painter.setBrush(sensorStateToColor(subState));
        m_painter.drawRect(QRectF(qRound(left) - 0.5, top, qRound(left + width) - qRound(left), height));
        left += width;
      }
      block.setHeight(block.height() - height);
    }

    if(!label.isEmpty())
    {
      m_painter.setPen(m_colorScheme.blockText);
      m_painter.drawText(block, label, QTextOption(Qt::AlignCenter));
    }
  }
  else
    assert(false);
}

void TilePainter::drawRailDecoupler(const QRectF& r, TileRotate rotate, bool isReserved, DecouplerState state)
{
  setTrackPen(isReserved);
  drawStraight(r, rotate);

  m_painter.save();
  m_painter.translate(r.center());
  m_painter.rotate(toDeg(rotate));
  m_painter.setPen(QPen(m_colorScheme.track, r.width() / 12, Qt::SolidLine, Qt::RoundCap));
  const qreal h = r.height() * 0.6;
  const qreal w = r.width() * 0.3;
  m_painter.setBrush(state == DecouplerState::Activated ? m_colorScheme.decouplerActivated : m_colorScheme.decouplerDeactivated);
  m_painter.drawRect(QRectF(-w / 2, -h / 2, w, h));
  m_painter.restore();
}

void TilePainter::drawRailNX(const QRectF& r, TileRotate rotate, bool isReserved, bool isEnabled, bool pressed)
{
  setTrackPen(isReserved);
  drawStraight(r, rotate);
  drawPushButton(r, pressed ? Color::White : (isEnabled ? Color::Blue : Color::Gray));
}

void TilePainter::drawLabel(const QRectF& r, TileRotate rotate, const QString& text, TextAlign textAlign, Color textColor, Color backgroundColor)
{
  m_painter.save();

  m_painter.setPen(Qt::NoPen);
  m_painter.setBrush(backgroundColor == Color::None ? m_colorScheme.background : toQColor(backgroundColor));
  m_painter.drawRect(r.adjusted(1, 1, -1, -1));

  if(!text.isEmpty())
  {
    m_painter.translate(r.center());
    m_painter.rotate(toDeg(rotate));
    m_painter.setPen(textColor == Color::None ? m_colorScheme.foreground : toQColor(textColor));
    QRectF textRect{r};
    if(rotate == TileRotate::Deg90 || rotate == TileRotate::Deg270)
    {
      textRect = textRect.transposed();
    }
    textRect.moveCenter({0., 0.});
    m_painter.drawText(textRect, text, QTextOption(toAlignment(textAlign)));
  }

  m_painter.restore();
}
