/**
 * client/src/board/boardareawidget.cpp
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

#include "boardareawidget.hpp"
#include <cmath>
#include <QPainter>
#include <QPaintEvent>
#include <QtMath>
#include <QApplication>
#include "boardwidget.hpp"
#include "tile.hpp"
#include "../network/board.hpp"
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
  const int trackWidth = tileSize / 5;
  const QPen trackPen(trackColor, trackWidth, Qt::SolidLine, Qt::FlatCap);
  const QPen turnoutStatePen(Qt::blue, (trackWidth + 1) / 2, Qt::SolidLine, Qt::FlatCap);
  const QPen blockPen{trackColor};
  const QBrush blockBrushFree{QColor{0x66, 0xC6, 0x66}};
  const QBrush blockBrushOccupied{QColor{0xC6, 0x66, 0x66}};

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
  const QRect tiles{viewport.left() / gridSize, viewport.top() / gridSize, viewport.width() / gridSize, viewport.height() / gridSize};

  for(auto it : m_board.board().tileData())
    if(it.first.x + it.second.width() - 1 >= tiles.left() && it.first.x <= tiles.right() &&
        it.first.y + it.second.height() - 1 >= tiles.top() && it.first.y <= tiles.bottom())
    {
      const TileRotate a = it.second.rotate();
      painter.setBrush(Qt::NoBrush);

      const QRect r = tileRect(it.first.x, it.first.y, tileSize);
      switch(it.second.id())
      {
        case TileId::RailStraight:
          painter.setPen(trackPen);
          Tile::drawStraight(painter, r, it.second.rotate());
          break;

        case TileId::RailCurve45:
          painter.setPen(trackPen);
          Tile::drawCurve45(painter, r, it.second.rotate());
          break;

        case TileId::RailCurve90:
          painter.setPen(trackPen);
          Tile::drawCurve90(painter, r, it.second.rotate());
          break;

        case TileId::RailCross45:
          painter.setPen(trackPen);
          Tile::drawStraight(painter, r, a);
          Tile::drawStraight(painter, r, a - TileRotate::Deg45);
          break;

        case TileId::RailCross90:
          painter.setPen(trackPen);
          Tile::drawStraight(painter, r, a);
          Tile::drawStraight(painter, r, a + TileRotate::Deg90);
          break;

        case TileId::RailTurnoutLeft:
          painter.setPen(trackPen);
          Tile::drawStraight(painter, r, it.second.rotate());
          Tile::drawCurve45(painter, r, it.second.rotate());
          painter.setPen(turnoutStatePen);
          Tile::drawCurve45(painter, r.adjusted(2, 2, -2, -2), it.second.rotate());
          break;

        case TileId::RailTurnoutRight:
          painter.setPen(trackPen);
          Tile::drawStraight(painter, r, it.second.rotate());
          Tile::drawCurve45(painter, r, it.second.rotate() + TileRotate::Deg225);
          painter.setPen(turnoutStatePen);
          Tile::drawStraight(painter, r.adjusted(2, 2, -2, -2), it.second.rotate());
          break;

        case TileId::RailTurnoutWye:
          painter.setPen(trackPen);
          Tile::drawCurve45(painter, r, it.second.rotate());
          Tile::drawCurve45(painter, r, it.second.rotate() + TileRotate::Deg225);
          painter.setPen(turnoutStatePen);
          Tile::drawCurve45(painter, r.adjusted(2, 2, -2, -2), it.second.rotate() + TileRotate::Deg225);
          break;

        case TileId::RailTurnout3Way:
          painter.setPen(trackPen);
          Tile::drawStraight(painter, r, a);
          Tile::drawCurve45(painter, r, a);
          Tile::drawCurve45(painter, r, a + TileRotate::Deg225);
          painter.setPen(turnoutStatePen);
          Tile::drawCurve45(painter, r.adjusted(2, 2, -2, -2), a + TileRotate::Deg225);
          break;

        case TileId::RailTurnoutSingleSlip:
          painter.setPen(trackPen);
          Tile::drawStraight(painter, r, a);
          Tile::drawStraight(painter, r, a - TileRotate::Deg45);
          Tile::drawCurve45(painter, r, a);
          painter.setPen(turnoutStatePen);
          Tile::drawCurve45(painter, r.adjusted(2, 2, -2, -2), a);
          break;

        case TileId::RailTurnoutDoubleSlip:
          painter.setPen(trackPen);
          Tile::drawStraight(painter, r, a);
          Tile::drawStraight(painter, r, a - TileRotate::Deg45);
          Tile::drawCurve45(painter, r, a);
          Tile::drawCurve45(painter, r, a + TileRotate::Deg180);
          painter.setPen(turnoutStatePen);
          Tile::drawCurve45(painter, r.adjusted(2, 2, -2, -2), a);
          Tile::drawCurve45(painter, r.adjusted(2, 2, -2, -2), a + TileRotate::Deg180);
          break;

        case TileId::RailSensor:
        {
          painter.setPen(trackPen);
          Tile::drawStraight(painter, r, it.second.rotate());
          painter.setPen(Qt::NoPen);
          painter.setBrush(trackPen.brush());
          int sz = r.width() / 4;
          painter.drawEllipse(r.adjusted(+sz, +sz, -sz, -sz));
          painter.setBrush(a == TileRotate::Deg0 ? Qt::red : Qt::darkGreen); // TODO: draw state
          sz += trackWidth / 2;
          painter.drawEllipse(r.adjusted(+sz, +sz, -sz, -sz));
          break;
        }
        case TileId::RailBufferStop:
          painter.setPen(trackPen);
          Tile::drawBufferStop(painter, r, a);
          break;

        case TileId::RailSignal2Aspect:
          painter.setPen(trackPen);
          Tile::drawStraight(painter, r, a);
          Tile::drawSignal2Aspect(painter, r, a);
          break;

        case TileId::RailSignal3Aspect:
          painter.setPen(trackPen);
          Tile::drawStraight(painter, r, a);
          Tile::drawSignal3Aspect(painter, r, a);
          break;

        case TileId::RailBlock:
        {
          //break;
          if(a == TileRotate::Deg0)
          {
            const QRectF tile = r.adjusted(0, 0, 0, (r.height() - 1) * (it.second.height() - 1));
            painter.setPen(trackPen);
            painter.drawLine(topCenter(tile), bottomCenter(tile));
            painter.setBrush(blockBrushOccupied);
            painter.setPen(blockPen);
            const qreal m = 0.5 + qFloor(tile.width() / 10);
            const QRectF block = tile.adjusted(m, m, -m, -m);
            painter.drawRect(block);
            const qreal inputs = 3;
            const qreal height = block.height() / inputs;
            const qreal width = qRound(block.width() / 5);
            qreal top = block.top();
            for(int i = 0; i < inputs; i++)
            {
              if(i != 1)
                painter.setBrush(blockBrushFree);
              else
                painter.setBrush(blockBrushOccupied);
              painter.drawRect(QRectF(block.left(), qRound(top) - 0.5, width, qRound(top + height) - qRound(top)));
              top += height;
            }
          }
          else if(a == TileRotate::Deg90)
          {
            const QRectF tile = r.adjusted(0, 0, (r.width() - 1) * (it.second.width() - 1), 0);
            painter.setPen(trackPen);
            painter.drawLine(centerLeft(tile), centerRight(tile));
            painter.setBrush(blockBrushOccupied);
            painter.setPen(blockPen);
            const qreal m = 0.5 + qFloor(tile.height() / 10);
            const QRectF block = tile.adjusted(m, m, -m, -m);
            painter.drawRect(block);
            const int inputs = 4;
            const qreal width = block.width() / inputs;
            const qreal height = qRound(block.height() / 5);
            const qreal top = block.bottom() - height;
            double left = block.left();
            for(int i = 0; i < inputs; i++)
            {
              if(i == 0)
                painter.setBrush(blockBrushFree);
              else
                painter.setBrush(blockBrushOccupied);
              painter.drawRect(QRectF(qRound(left) - 0.5, top, qRound(left + width) - qRound(left), height));
              left += width;
            }
          }
          else
            Q_ASSERT(false);

          break;
        }
        case TileId::None:
        case TileId::ReservedForFutureExpension:
          break;
      }
    }
}
