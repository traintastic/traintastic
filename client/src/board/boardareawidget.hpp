/**
 * client/src/board/boardareawidget.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_BOARD_BOARDAREAWIDGET_HPP
#define TRAINTASTIC_CLIENT_BOARD_BOARDAREAWIDGET_HPP

#include <QWidget>
#include <traintastic/board/tileid.hpp>
#include <traintastic/board/tilelocation.hpp>
#include <traintastic/enum/tilerotate.hpp>
#include <traintastic/enum/decouplerstate.hpp>
#include <traintastic/enum/directioncontrolstate.hpp>
#include <traintastic/enum/sensorstate.hpp>
#include <traintastic/enum/signalaspect.hpp>
#include <traintastic/enum/tristate.hpp>
#include <traintastic/enum/turnoutposition.hpp>
#include <traintastic/enum/color.hpp>
#include "boardcolorscheme.hpp"
#include "../network/abstractproperty.hpp"
#include "../network/objectptr.hpp"

class BoardWidget;

class BoardAreaWidget : public QWidget
{
  Q_OBJECT

  public:
    enum class Grid
    {
      None = 0,
      Line,
      Dot,
    };

    enum class MouseMoveAction
    {
      None,
      AddTile,
      MoveTile,
      ResizeTile,
    };

  private:
    const BoardColorScheme* m_colorScheme;

  protected:
    static constexpr int boardMargin = 1; // tile

    BoardWidget& m_board;
    AbstractProperty* m_boardLeft;
    AbstractProperty* m_boardTop;
    AbstractProperty* m_boardRight;
    AbstractProperty* m_boardBottom;
    Grid m_grid;
    int m_zoomLevel;

    bool m_mouseLeftButtonPressed;
    TileLocation m_mouseLeftButtonPressedTileLocation;
    bool m_mouseRightButtonPressed;
    QPoint m_mouseRightButtonPressedPoint;

    MouseMoveAction m_mouseMoveAction;
    TileId m_mouseMoveTileId;
    TileLocation m_mouseMoveTileLocation;
    TileRotate m_mouseMoveTileRotate;
    uint8_t m_mouseMoveTileWidth;
    uint8_t m_mouseMoveTileHeight;
    TileLocation m_mouseMoveHideTileLocation;
    uint8_t m_mouseMoveTileWidthMax;
    uint8_t m_mouseMoveTileHeightMax;

    inline int boardLeft() const { return Q_LIKELY(m_boardLeft) ? m_boardLeft->toInt() - boardMargin : 0; }
    inline int boardTop() const { return Q_LIKELY(m_boardTop) ? m_boardTop->toInt() - boardMargin: 0; }
    inline int boardRight() const { return Q_LIKELY(m_boardRight) ? m_boardRight->toInt() + boardMargin: 0; }
    inline int boardBottom() const { return Q_LIKELY(m_boardBottom) ? m_boardBottom->toInt() + boardMargin: 0; }

    static constexpr int getTileSize(int zoomLevel) { return 25 + zoomLevel * 5; }
    int getTileSize() const { return getTileSize(m_zoomLevel); }
    TurnoutPosition getTurnoutPosition(const TileLocation& l) const;
    SensorState getSensorState(const TileLocation& l) const;
    DirectionControlState getDirectionControlState(const TileLocation& l) const;
    SignalAspect getSignalAspect(const TileLocation& l) const;
    Color getColor(const TileLocation& l) const;
    DecouplerState getDecouplerState(const TileLocation& l) const;
    TileLocation pointToTileLocation(const QPoint& p);

    void leaveEvent(QEvent *event) final;
    void keyPressEvent(QKeyEvent* event) final;
    void mousePressEvent(QMouseEvent* event) final;
    void mouseReleaseEvent(QMouseEvent* event) final;
    void mouseMoveEvent(QMouseEvent* event) final;
    void wheelEvent(QWheelEvent* event) final;
    void paintEvent(QPaintEvent* event) final;

  protected slots:
    void settingsChanged();
    void updateMinimumSize();

  public:
    static constexpr int zoomLevelMin = -2;
    static constexpr int zoomLevelMax = 15;

    BoardAreaWidget(BoardWidget& board, QWidget* parent = nullptr);

    Grid grid() const { return m_grid; }
    void nextGrid();
    int zoomLevel() const { return m_zoomLevel; }
    float zoomRatio() const { return static_cast<float>(getTileSize()) / getTileSize(0); }

    void setMouseMoveAction(MouseMoveAction action);
    void setMouseMoveTileId(TileId id);
    TileRotate mouseMoveTileRotate() const { return m_mouseMoveTileRotate; }
    void setMouseMoveTileRotate(TileRotate rotate);
    uint8_t mouseMoveTileHeight() const { return m_mouseMoveTileHeight; }
    uint8_t mouseMoveTileWidth() const { return m_mouseMoveTileWidth; }
    void setMouseMoveTileSize(uint8_t x, uint8_t y);
    void setMouseMoveHideTileLocation(TileLocation l);
    void setMouseMoveTileSizeMax(uint8_t width, uint8_t height);

  public slots:
    void tileObjectAdded(int16_t x, int16_t y, const ObjectPtr& object);
    void setGrid(Grid value);
    void setZoomLevel(int value);
    void zoomIn() { setZoomLevel(zoomLevel() + 1); }
    void zoomOut() { setZoomLevel(zoomLevel() - 1); }

  signals:
    void gridChanged(Grid);
    void zoomLevelChanged(int);
    void tileClicked(int16_t x, int16_t y);
    void rightClicked();
    void mouseTileLocationChanged(int16_t x, int16_t y);
};

#endif
