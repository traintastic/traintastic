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

#ifndef TRAINTASTIC_CLIENT_BOARD_TILEPAINTER_HPP
#define TRAINTASTIC_CLIENT_BOARD_TILEPAINTER_HPP

#include <array>
#include <vector>
#include <QPainter>
#include <traintastic/board/tileid.hpp>
#include <traintastic/enum/tilerotate.hpp>
#include <traintastic/enum/blockstate.hpp>
#include <traintastic/enum/crossstate.hpp>
#include <traintastic/enum/decouplerstate.hpp>
#include <traintastic/enum/directioncontrolstate.hpp>
#include <traintastic/enum/sensorstate.hpp>
#include <traintastic/enum/signalaspect.hpp>
#include <traintastic/enum/tristate.hpp>
#include <traintastic/enum/turnoutposition.hpp>
#include "../enum/color.hpp"
#include "../enum/textalign.hpp"
#include "../network/objectptr.hpp"

struct BoardColorScheme;

class TilePainter
{
  private:
    inline static const QColor signalRed{192, 0, 0};
    inline static const QColor signalYellow{192, 192, 32};
    inline static const QColor signalGreen{0, 192, 0};

    const BoardColorScheme& m_colorScheme;
    const bool m_showBlockSensorStates;
    const bool m_turnoutDrawState;
    const int m_trackWidth;
    const int m_turnoutMargin;
    const QPen m_blockPen;
    const QPen m_trackPen;
    const QPen m_trackDisabledPen;
    const QPen m_trackReservedPen;
    const QPen m_trackReservedDisabledPen;
    const QPen m_trackErasePen;
    const QPen m_turnoutPen;
    const QPen m_turnoutStatePen;

    QPainter& m_painter;

    inline void setTrackPen() { m_painter.setPen(m_trackPen); }
    inline void setTrackPen(bool isReserved) { m_painter.setPen(isReserved ? m_trackReservedPen : m_trackPen); }
    inline void setTrackDisabledPen() { m_painter.setPen(m_trackDisabledPen); }
    inline void setTrackErasePen() { m_painter.setPen(m_trackErasePen); }
    inline void setTurnoutPen() { m_painter.setPen(m_turnoutDrawState ? m_trackPen : m_trackDisabledPen); }
    inline void setTurnoutStatePen() { m_painter.setPen(m_turnoutDrawState ? m_turnoutStatePen : m_trackPen); }
    inline QRectF turnoutStateRect(const QRectF& r) { return m_turnoutDrawState ? r.adjusted(m_turnoutMargin, m_turnoutMargin, -m_turnoutMargin, -m_turnoutMargin) : r; }

    QColor sensorStateToColor(SensorState value) const;
    void setBlockStateBrush(BlockState value);

    void drawStraight(const QRectF& r, TileRotate rotate);
    void drawCurve45(const QRectF& r, TileRotate rotate);
    void drawCurve90(QRectF r, TileRotate rotate);

    void drawBufferStop(const QRectF& r, TileRotate rotate);
    void drawLink(const QRectF& r, TileRotate rotate);

    void drawTriangle(const QRectF& r);
    void drawLED(const QRectF& r, const QColor& color, const QColor& borderColor);

    void drawTurnoutStandard(TileId id, const QRectF& r, TileRotate rotate, TurnoutPosition reservedPosition = TurnoutPosition::Unknown, TurnoutPosition position = TurnoutPosition::Unknown);
    void drawTurnoutSlip(TileId id, const QRectF& r, TileRotate rotate, TurnoutPosition reservedPosition = TurnoutPosition::Unknown, TurnoutPosition position = TurnoutPosition::Unknown);

    void drawSignal2Aspect(QRectF r, TileRotate rotate, SignalAspect aspect);
    void drawSignal3Aspect(QRectF r, TileRotate rotate, SignalAspect aspect);
    void drawSignalDirection(QRectF r, TileRotate rotate);

    void drawRailBlock(const QRectF& r, TileRotate rotate, bool isReservedA = false, bool isReservedB = false, const ObjectPtr& blockTile = {});

  public:
    TilePainter(QPainter& painter, int tileSize, const BoardColorScheme& colorScheme);

    void draw(TileId id, const QRectF& r, TileRotate rotate, bool isReserved = false);
    void drawBridge(TileId id, const QRectF& r, TileRotate rotate, bool isReservedAC = false, bool isReservedBD = false);
    void drawCross(TileId id, const QRectF& r, TileRotate rotate, CrossState reservedState = CrossState::Unset);
    void drawSensor(TileId id, const QRectF& r, TileRotate rotate, bool isReserved = false, SensorState state = SensorState::Unknown);
    void drawDirectionControl(TileId id, const QRectF& r, TileRotate rotate, bool isReserved = false, DirectionControlState state = DirectionControlState::Both);
    void drawTurnout(TileId id, const QRectF& r, TileRotate rotate, TurnoutPosition reservedPosition = TurnoutPosition::Unknown, TurnoutPosition position = TurnoutPosition::Unknown);
    void drawSignal(TileId id, const QRectF& r, TileRotate rotate, bool isReserved = false, SignalAspect aspect = SignalAspect::Unknown);
    void drawBlock(TileId id, const QRectF& r, TileRotate rotate, bool isReservedA = false, bool isReservedB = false, const ObjectPtr& blockTile = {});

    void drawPushButton(const QRectF& r, Color color = Color::Yellow, Color textColor = Color::Black, const QString& text = {});
    void drawSwitch(const QRectF& r, bool value = false, Color colorOn = Color::Yellow, Color colorOff = Color::Gray);

    void drawRailDecoupler(const QRectF& r, TileRotate rotate, bool isReserved = false, DecouplerState active = DecouplerState::Deactivated);

    void drawRailNX(const QRectF& r, TileRotate rotate, bool isReserved = false, bool isEnabled = false, bool pressed = false);

    void drawLabel(const QRectF& r, TileRotate rotate, const QString& text = "txt", TextAlign textAlign = TextAlign::Center, Color textColor = Color::None, Color backgroundColor = Color::None);
};

#endif
