/**
 * client/src/board/boardareawidget.hpp
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

#ifndef TRAINTASTIC_CLIENT_BOARD_BOARDAREAWIDGET_HPP
#define TRAINTASTIC_CLIENT_BOARD_BOARDAREAWIDGET_HPP

#include <QWidget>
#include <traintastic/board/tileid.hpp>
#include <traintastic/board/tilelocation.hpp>
#include <traintastic/board/tilerotate.hpp>
#include <traintastic/enum/blockstate.hpp>
#include <traintastic/enum/sensorstate.hpp>
#include <traintastic/enum/signalaspect.hpp>
#include <traintastic/enum/tristate.hpp>
#include <traintastic/enum/turnoutposition.hpp>
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

    TileId m_mouseMoveTileId;
    TileLocation m_mouseMoveTileLocation;
    TileRotate m_mouseMoveTileRotate;

    inline int boardLeft() const { return Q_LIKELY(m_boardLeft) ? m_boardLeft->toInt() - boardMargin : 0; }
    inline int boardTop() const { return Q_LIKELY(m_boardTop) ? m_boardTop->toInt() - boardMargin: 0; }
    inline int boardRight() const { return Q_LIKELY(m_boardRight) ? m_boardRight->toInt() + boardMargin: 0; }
    inline int boardBottom() const { return Q_LIKELY(m_boardBottom) ? m_boardBottom->toInt() + boardMargin: 0; }

    int getTileSize() const { return 25 + m_zoomLevel * 5; }
    TurnoutPosition getTurnoutPosition(const TileLocation& l) const;
    BlockState getBlockState(const TileLocation& l) const;
    std::vector<SensorState> getBlockSensorStates(const TileLocation& l) const;
    SensorState getSensorState(const TileLocation& l) const;
    SignalAspect getSignalAspect(const TileLocation& l) const;
    TileLocation pointToTileLocation(const QPoint& p);

    void leaveEvent(QEvent *event) final;
    void keyPressEvent(QKeyEvent* event) final;
    void mousePressEvent(QMouseEvent* event) final;
    void mouseReleaseEvent(QMouseEvent* event) final;
    void mouseMoveEvent(QMouseEvent* event) final;
    void wheelEvent(QWheelEvent* event) final;
    void paintEvent(QPaintEvent* event) final;

  protected slots:
    void updateMinimumSize();

  public:
    static constexpr int zoomLevelMin = 0;
    static constexpr int zoomLevelMax = 15;

    BoardAreaWidget(BoardWidget& board, QWidget* parent = nullptr);

    Grid grid() const { return m_grid; }
    void nextGrid();
    int zoomLevel() const { return m_zoomLevel; }

    void setMouseMoveTileId(TileId id);
    void setMouseMoveTileRotate(TileRotate rotate);

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
