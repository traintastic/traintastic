/**
 * client/src/board/boardareawidget.cpp
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

#include "boardareawidget.hpp"
#include <cmath>
#include <QPainter>
#include <QPaintEvent>
#include <QtMath>
#include <QApplication>
#include "boardwidget.hpp"
#include "tile.hpp"
#include "../network/board.hpp"
#include "../network/abstractproperty.hpp"
#include "../utils/rectf.hpp"

QRect rectToViewport(const QRect& r, const int gridSize)
{
  QRect viewport;
  viewport.setLeft((r.left() / gridSize) * gridSize);
  viewport.setTop((r.top() / gridSize) * gridSize);
  viewport.setRight(((r.right() + gridSize - 1) / gridSize) * gridSize);
  viewport.setBottom(((r.bottom() + gridSize - 1) / gridSize) * gridSize);
  return viewport;
}

constexpr QRect tileRect(const int x, const int y, const int tileSize)
{
  return QRect{x * (tileSize - 1), y * (tileSize - 1), tileSize, tileSize};
}


BoardAreaWidget::BoardAreaWidget(BoardWidget& board, QWidget* parent) :
  QWidget(parent),
  m_board{board},
  m_grid{Grid::Dot},
  m_zoomLevel{0}
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setFocusPolicy(Qt::StrongFocus);
  setMinimumSize(2000, 2000); // TODO: get size from board
}

void BoardAreaWidget::setGrid(Grid value)
{
  if(m_grid != value)
  {
    m_grid = value;
    update();
    emit gridChanged(m_grid);
  }
}

void BoardAreaWidget::nextGrid()
{
  switch(grid())
  {
    case Grid::None:
      setGrid(Grid::Dot);
      break;

    case Grid::Dot:
      setGrid(Grid::Line);
      break;

    case Grid::Line:
      setGrid(Grid::None);
      break;
  }
}

void BoardAreaWidget::setZoomLevel(int value)
{
  value = std::clamp(value, zoomLevelMin, zoomLevelMax);
  if(m_zoomLevel != value)
  {
    m_zoomLevel = value;
    // TODO: updateMinimumSize();
    update();
    emit zoomLevelChanged(m_zoomLevel);
  }
}

TurnoutPosition BoardAreaWidget::getTurnoutPosition(const TileLocation& l) const
{
  if(ObjectPtr object = m_board.board().getTileObject(l))
    if(const auto* p = object->getProperty("position"))
      return p->toEnum<TurnoutPosition>();
  return TurnoutPosition::Unknown;
}

SignalAspect BoardAreaWidget::getSignalAspect(const TileLocation& l) const
{
  if(ObjectPtr object = m_board.board().getTileObject(l))
    if(const auto* p = object->getProperty("aspect"))
      return p->toEnum<SignalAspect>();
  return SignalAspect::Unknown;
}

TileLocation BoardAreaWidget::pointToTileLocation(const QPoint& p)
{
  const int pxPerTile = getTileSize() - 1;
  return TileLocation{static_cast<int16_t>(p.x() / pxPerTile), static_cast<int16_t>(p.y() / pxPerTile)};
}

void BoardAreaWidget::keyPressEvent(QKeyEvent* event)
{
  if(event->key() == Qt::Key_G && event->modifiers() == Qt::ControlModifier)
    nextGrid();
  else if(event->matches(QKeySequence::ZoomIn))
    zoomIn();
  else if(event->matches(QKeySequence::ZoomOut))
    zoomOut();
  else
    QWidget::keyPressEvent(event);
}

void BoardAreaWidget::mousePressEvent(QMouseEvent* event)
{
  if(event->button() == Qt::LeftButton)
  {
    m_mouseLeftButtonPressed = true;
    m_mouseLeftButtonPressedTileLocation = pointToTileLocation(event->pos());
  }
  else if(event->button() == Qt::RightButton)
  {
    m_mouseRightButtonPressed = true;
    m_mouseRightButtonPressedPoint = event->pos();
  }
}

void BoardAreaWidget::mouseReleaseEvent(QMouseEvent* event)
{
  if(m_mouseLeftButtonPressed && event->button() == Qt::LeftButton)
  {
    m_mouseLeftButtonPressed = false;
    TileLocation tl = pointToTileLocation(event->pos());

    if(m_mouseLeftButtonPressedTileLocation == tl) // click
      emit tileClicked(tl.x, tl.y);
  }
  else if(m_mouseRightButtonPressed && event->button() == Qt::RightButton)
  {
    m_mouseRightButtonPressed = false;
    if((event->pos() - m_mouseRightButtonPressedPoint).manhattanLength() < 5)
      emit rightClicked();
  }
}

void BoardAreaWidget::mouseMoveEvent(QMouseEvent* event)
{
  if(hasMouseTracking())
  {
    TileLocation tl = pointToTileLocation(event->pos());
    if(m_mouseMoveTileLocation != tl)
    {
      m_mouseMoveTileLocation = tl;
      emit mouseTileLocationChanged(tl.x, tl.y);
    }
  }
  QWidget::mouseMoveEvent(event);
}

void BoardAreaWidget::wheelEvent(QWheelEvent* event)
{
  if(QApplication::keyboardModifiers() == Qt::ControlModifier && event->angleDelta().y() != 0)
  {
    if(event->angleDelta().y() < 0)
      zoomOut();
    else
      zoomIn();
    event->accept();
  }
  else
    QWidget::wheelEvent(event);
}

void BoardAreaWidget::paintEvent(QPaintEvent* event)
{
  const QColor backgroundColor{0x10, 0x10, 0x10};
  const QColor gridColor{0x40, 0x40, 0x40};
  const QColor trackColor{0xC0, 0xC0, 0xC0};
  const int tileSize = getTileSize();
  const int gridSize = tileSize - 1;

  QPainter painter(this);

  const QRect viewport = rectToViewport(event->rect(), gridSize);

  painter.fillRect(viewport, backgroundColor);

  // draw grid:
  switch(m_grid)
  {
    case Grid::None:
      break;

    case Grid::Line:
      painter.setPen(gridColor);
      for(int y = viewport.top(); y <= viewport.bottom(); y += gridSize)
        painter.drawLine(viewport.left(), y, viewport.right(), y);
      for(int x = viewport.left(); x <= viewport.right(); x += gridSize)
        painter.drawLine(x, viewport.top(), x, viewport.bottom());
      break;

    case Grid::Dot:
      painter.setPen(gridColor);
      for(int y = viewport.top(); y <= viewport.bottom(); y += gridSize)
        for(int x = viewport.left(); x <= viewport.right(); x += gridSize)
          painter.drawPoint(x, y);
      break;
  }

  painter.setRenderHint(QPainter::Antialiasing, true);

  // draw tiles:
  TilePainter tilePainter{painter, tileSize};

  const QRect tiles{viewport.left() / gridSize, viewport.top() / gridSize, viewport.width() / gridSize, viewport.height() / gridSize};

  for(auto it : m_board.board().tileData())
    if(it.first.x + it.second.width() - 1 >= tiles.left() && it.first.x <= tiles.right() &&
        it.first.y + it.second.height() - 1 >= tiles.top() && it.first.y <= tiles.bottom())
    {
      const TileId id = it.second.id();
      const TileRotate a = it.second.rotate();
      painter.setBrush(Qt::NoBrush);

      const QRectF r = tileRect(it.first, it.second.width(), it.second.height(), tileSize);
      switch(id)
      {
        case TileId::RailStraight:
        case TileId::RailCurve45:
        case TileId::RailCurve90:
        case TileId::RailCross45:
        case TileId::RailCross90:
        case TileId::RailBufferStop:
          tilePainter.draw(id, r, a);
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
          tilePainter.drawTurnout(id, r, a, getTurnoutPosition(it.first));
          break;

        case TileId::RailSensor:
          tilePainter.drawSensor(id, r, a); // TODO: add state
          break;

        case TileId::RailSignal2Aspect:
        case TileId::RailSignal3Aspect:
          tilePainter.drawSignal(id, r, a, getSignalAspect(it.first));
          break;

        case TileId::RailBlock:
          tilePainter.drawBlock(id, r, a);
          break;

        case TileId::None:
        case TileId::ReservedForFutureExpension:
          break;
      }
    }
}
