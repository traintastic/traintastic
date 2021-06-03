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

#ifndef TRAINTASTIC_CLIENT_BOARD_TILEPAINTER_HPP
#define TRAINTASTIC_CLIENT_BOARD_TILEPAINTER_HPP

#include <vector>
#include <QPainter>
#include <traintastic/board/tileid.hpp>
#include <traintastic/board/tilerotate.hpp>
#include <traintastic/enum/signalaspect.hpp>
#include <traintastic/enum/tristate.hpp>
#include <traintastic/enum/turnoutposition.hpp>

class TilePainter
{
  private:


    inline static const QColor trackColor{0xC0, 0xC0, 0xC0};
    inline static const QColor signalRed{192, 0, 0};
    inline static const QColor signalYellow{192, 192, 32};
    inline static const QColor signalGreen{0, 192, 0};
    inline static const QBrush blockBrushFree{QColor{0x66, 0xC6, 0x66}};
    inline static const QBrush blockBrushOccupied{QColor{0xC6, 0x66, 0x66}};
    inline static const QBrush blockBrushUnknown{QColor{0x66, 0x66, 0x66}};
    inline static const QPen blockPen{trackColor};

    const int m_trackWidth;
    const int m_turnoutMargin;
    const QPen m_trackPen;
    const QPen m_turnoutStatePen;

    QPainter& m_painter;

    inline void setTrackPen() { m_painter.setPen(m_trackPen); }
    inline void setTurnoutStatePen() { m_painter.setPen(m_turnoutStatePen); }
    inline QRectF turnoutStateRect(const QRectF& r) { return r.adjusted(m_turnoutMargin, m_turnoutMargin, -m_turnoutMargin, -m_turnoutMargin); }

    void setBlockStateBrush(TriState value);

    void drawStraight(const QRectF& r, TileRotate rotate);
    void drawCurve45(const QRectF& r, TileRotate rotate);
    void drawCurve90(QRectF r, TileRotate rotate);

    void drawBufferStop(const QRectF& r, TileRotate rotate);

    void drawSignal2Aspect(QRectF r, TileRotate rotate, SignalAspect aspect);
    void drawSignal3Aspect(QRectF r, TileRotate rotate, SignalAspect aspect);

    void drawRailBlock(const QRectF& r, TileRotate rotate, TriState state = TriState::Undefined, const std::vector<TriState> subStates = {});

  public:
    TilePainter(QPainter& painter, int tileSize);

    void draw(TileId id, const QRectF& r, TileRotate rotate);
    void drawSensor(TileId id, const QRectF& r, TileRotate rotate, TriState state = TriState::Undefined);
    void drawTurnout(TileId id, const QRectF& r, TileRotate rotate, TurnoutPosition position = TurnoutPosition::Unknown);
    void drawSignal(TileId id, const QRectF& r, TileRotate rotate, SignalAspect aspect = SignalAspect::Unknown);
    void drawBlock(TileId id, const QRectF& r, TileRotate rotate, TriState state = TriState::Undefined, const std::vector<TriState> subStates = {});
};

#endif
