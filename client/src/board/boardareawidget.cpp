/**
 * client/src/board/boardareawidget.cpp
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

#include "boardareawidget.hpp"
#include <cmath>
#include <QPainter>
#include <QPaintEvent>
#include <QtMath>
#include <QApplication>
#include <QToolTip>
#include <traintastic/locale/locale.hpp>
#include "boardwidget.hpp"
#include "getboardcolorscheme.hpp"
#include "tilepainter.hpp"
#include "blockhighlight.hpp"
#include "../mainwindow.hpp"
#include "../network/board.hpp"
#include "../network/callmethod.hpp"
#include "../network/object.tpp"
#include "../network/object/blockrailtile.hpp"
#include "../network/object/nxbuttonrailtile.hpp"
#include "../network/abstractproperty.hpp"
#include "../network/abstractvectorproperty.hpp"
#include "../utils/enum.hpp"
#include "../utils/rectf.hpp"
#include "../misc/mimedata.hpp"
#include "../settings/boardsettings.hpp"

QRect rectToViewport(const QRect& r, const int gridSize)
{
  QRect viewport;
  viewport.setLeft((r.left() / gridSize) * gridSize);
  viewport.setTop((r.top() / gridSize) * gridSize);
  viewport.setRight(((r.right() + gridSize - 1) / gridSize) * gridSize);
  viewport.setBottom(((r.bottom() + gridSize - 1) / gridSize) * gridSize);
  return viewport;
}

// excludes grid/border
constexpr QRectF drawTileRect(const int x, const int y, const int w, const int h, const int tileSize)
{
  return QRectF(x * (tileSize - 1), y * (tileSize - 1), 1 + w * (tileSize - 1), 1 + h * (tileSize - 1));
}

// includes grid/border
constexpr QRect updateTileRect(const int x, const int y, const int w, const int h, const int tileSize)
{
  return QRect(x * (tileSize - 1) - 1, y * (tileSize - 1) - 1, 3 + w * (tileSize - 1), 3 + h * (tileSize - 1));
}


BoardAreaWidget::BoardAreaWidget(std::shared_ptr<Board> board, QWidget* parent) :
  QWidget(parent),
  m_colorScheme{&BoardColorScheme::dark},
  m_board{std::move(board)},
  m_boardLeft{m_board->getProperty("left")},
  m_boardTop{m_board->getProperty("top")},
  m_boardRight{m_board->getProperty("right")},
  m_boardBottom{m_board->getProperty("bottom")},
  m_grid{BoardAreaGrid::None},
  m_zoomLevel{0},
  m_blockHighlight{MainWindow::instance->blockHighlight()},
  m_mouseLeftButtonPressed{false},
  m_mouseRightButtonPressed{false},
  m_mouseMoveAction{MouseMoveAction::None},
  m_mouseMoveTileId{TileId::None},
  m_mouseMoveTileRotate{TileRotate::Deg0}
  , m_mouseMoveTileWidth{1}
  , m_mouseMoveTileHeight{1}
{
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
  setFocusPolicy(Qt::StrongFocus);
  setAcceptDrops(true);

  if(Q_LIKELY(m_boardLeft))
    connect(m_boardLeft, &AbstractProperty::valueChanged, this, &BoardAreaWidget::updateMinimumSize);
  if(Q_LIKELY(m_boardTop))
    connect(m_boardTop, &AbstractProperty::valueChanged, this, &BoardAreaWidget::updateMinimumSize);
  if(Q_LIKELY(m_boardRight))
    connect(m_boardRight, &AbstractProperty::valueChanged, this, &BoardAreaWidget::updateMinimumSize);
  if(Q_LIKELY(m_boardBottom))
    connect(m_boardBottom, &AbstractProperty::valueChanged, this, &BoardAreaWidget::updateMinimumSize);

  connect(&BoardSettings::instance(), &SettingsBase::changed, this, &BoardAreaWidget::settingsChanged);

  connect(&m_blockHighlight, &BlockHighlight::colorsChanged, this,
    [this](const QString& /*blockId*/, const QVector<Color>& /*colors*/)
    {
      update();
    });

  for(const auto& [l, object] : m_board->tileObjects())
    tileObjectAdded(l.x, l.y, object);

  settingsChanged();
  updateMinimumSize();
}

void BoardAreaWidget::tileObjectAdded(int16_t x, int16_t y, const ObjectPtr& object)
{
  const TileLocation l{x, y};

  auto handler =
    [this, l]()
    {
      try
      {
        const TileData& tileData = m_board->tileData().at(l);
        update(updateTileRect(l.x - boardLeft(), l.y - boardTop(), tileData.width(), tileData.height(), getTileSize()));
      }
      catch(...)
      {
      }
    };

  auto tryConnect =
    [this, handler, &object](const QString& name)
    {
      if(auto* property = dynamic_cast<BaseProperty*>(object->getInterfaceItem(name)))
        connect(property, &BaseProperty::valueChanged, this, handler);
    };

  switch(m_board->getTileId(l))
  {
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
      tryConnect("position");
      break;

    case TileId::RailSignal2Aspect:
    case TileId::RailSignal3Aspect:
      tryConnect("aspect");
      break;

    case TileId::RailSensor:
    case TileId::RailDirectionControl:
    case TileId::RailDecoupler:
      tryConnect("state");
      break;

    case TileId::RailBlock:
      tryConnect("name");
      tryConnect("state");
      tryConnect("sensor_states");
      if(auto* block = dynamic_cast<BlockRailTile*>(object.get())) /*[[likely]]*/
        connect(block, &BlockRailTile::trainsChanged, this, handler);
      break;

    case TileId::PushButton:
      tryConnect("color");
      tryConnect("text");
      tryConnect("text_color");
      break;

    case TileId::RailNXButton:
      tryConnect("enabled");
      if(auto* nxButton = dynamic_cast<NXButtonRailTile*>(object.get())) /*[[likely]]*/
      {
        connect(nxButton, &NXButtonRailTile::isPressedChanged, this, handler);
      }
      break;

    case TileId::Switch:
      tryConnect("color_off");
      tryConnect("color_on");
      tryConnect("value");
      break;

    case TileId::None:
    case TileId::RailStraight:
    case TileId::RailCurve45:
    case TileId::RailCurve90:
    case TileId::RailCross45:
    case TileId::RailCross90:
    case TileId::RailBufferStop:
    case TileId::RailBridge45Left:
    case TileId::RailBridge45Right:
    case TileId::RailBridge90:
    case TileId::RailTunnel:
    case TileId::RailOneWay:
    case TileId::RailLink:
    case TileId::HiddenRailCrossOver:
    case TileId::ReservedForFutureExpension:
      break;

    case TileId::Label:
      tryConnect("background_color");
      tryConnect("text");
      tryConnect("text_align");
      tryConnect("text_color");
      break;
  }
}

void BoardAreaWidget::setZoomLevel(int value)
{
  value = std::clamp(value, zoomLevelMin, zoomLevelMax);
  if(m_zoomLevel != value)
  {
    m_zoomLevel = value;
    updateMinimumSize();
    update();
    emit zoomLevelChanged(m_zoomLevel);
  }
}

void BoardAreaWidget::setMouseMoveAction(MouseMoveAction action)
{
  if(m_mouseMoveAction == action)
    return;

  m_mouseMoveAction = action;

  // reset others:
  m_mouseMoveTileId = TileId::None;
  m_mouseMoveTileRotate = TileRotate::Deg0;
  m_mouseMoveTileWidth = 1;
  m_mouseMoveTileHeight = 1;
  m_mouseMoveHideTileLocation = TileLocation::invalid;
  m_mouseMoveTileHeightMax = 1;
  m_mouseMoveTileHeightMax = 1;

  update();
}

void BoardAreaWidget::setMouseMoveTileId(TileId id)
{
  if(m_mouseMoveTileId == id)
    return;
  m_mouseMoveTileId = id;
  update();
}

void BoardAreaWidget::setMouseMoveTileRotate(TileRotate rotate)
{
  if(m_mouseMoveTileRotate == rotate)
    return;
  m_mouseMoveTileRotate = rotate;
  update();
}

void BoardAreaWidget::setMouseMoveTileSize(uint8_t w, uint8_t h)
{
  if(m_mouseMoveTileWidth == w && m_mouseMoveTileHeight == h)
    return;
  m_mouseMoveTileWidth = w;
  m_mouseMoveTileHeight = h;
  update();
}

void BoardAreaWidget::setMouseMoveHideTileLocation(TileLocation l)
{
  if(m_mouseMoveHideTileLocation == l)
    return;
  m_mouseMoveHideTileLocation = l;
  update();
}

void BoardAreaWidget::setMouseMoveTileSizeMax(uint8_t width, uint8_t height)
{
  if(m_mouseMoveTileWidthMax == width && m_mouseMoveTileHeightMax == height)
    return;
  m_mouseMoveTileWidthMax = width;
  m_mouseMoveTileHeightMax = height;
  update();
}

TurnoutPosition BoardAreaWidget::getTurnoutPosition(const TileLocation& l) const
{
  if(ObjectPtr object = m_board->getTileObject(l))
    if(const auto* p = object->getProperty("position"))
      return p->toEnum<TurnoutPosition>();
  return TurnoutPosition::Unknown;
}

SensorState BoardAreaWidget::getSensorState(const TileLocation& l) const
{
  if(ObjectPtr object = m_board->getTileObject(l))
    if(const auto* p = object->getProperty("state"))
      return p->toEnum<SensorState>();
  return SensorState::Unknown;
}

DirectionControlState BoardAreaWidget::getDirectionControlState(const TileLocation& l) const
{
  if(ObjectPtr object = m_board->getTileObject(l))
    if(const auto* p = object->getProperty("state"))
      return p->toEnum<DirectionControlState>();
  return DirectionControlState::Both;
}

SignalAspect BoardAreaWidget::getSignalAspect(const TileLocation& l) const
{
  if(ObjectPtr object = m_board->getTileObject(l))
    if(const auto* p = object->getProperty("aspect"))
      return p->toEnum<SignalAspect>();
  return SignalAspect::Unknown;
}

Color BoardAreaWidget::getColor(const TileLocation& l) const
{
  if(ObjectPtr object = m_board->getTileObject(l))
    if(const auto* p = object->getProperty("color"))
      return p->toEnum<Color>();
  return Color::None;
}

DecouplerState BoardAreaWidget::getDecouplerState(const TileLocation& l) const
{
  if(ObjectPtr object = m_board->getTileObject(l))
    if(const auto* p = object->getProperty("state"))
      return p->toEnum<DecouplerState>();
  return DecouplerState::Deactivated;
}

bool BoardAreaWidget::getNXButtonEnabled(const TileLocation& l) const
{
  if(auto object = std::dynamic_pointer_cast<NXButtonRailTile>(m_board->getTileObject(l)))
  {
    return object->getPropertyValueBool("enabled", false);
  }
  return false;
}

bool BoardAreaWidget::getNXButtonPressed(const TileLocation& l) const
{
  if(auto object = std::dynamic_pointer_cast<NXButtonRailTile>(m_board->getTileObject(l)))
  {
    return object->isPressed();
  }
  return false;
}

TileLocation BoardAreaWidget::pointToTileLocation(const QPoint& p)
{
  const int pxPerTile = getTileSize() - 1;
  return TileLocation{static_cast<int16_t>(p.x() / pxPerTile + boardLeft()), static_cast<int16_t>(p.y() / pxPerTile + boardTop())};
}

QString BoardAreaWidget::getTileToolTip(const TileLocation& l) const
{
  const auto tileId = m_board->getTileId(l);

  if(isRailTurnout(tileId))
  {
    if(auto turnout = m_board->getTileObject(l))
    {
      QString text = "<b>" + turnout->getPropertyValueString("name") + "</b>";
      if(auto* position = turnout->getProperty("position")) /*[[likely]]*/
      {
        text.append("<br>" + position->displayName() + ": " + translateEnum(*position));
      }
      return text;
    }
  }
  else if(isRailSignal(tileId))
  {
    if(auto signal = m_board->getTileObject(l))
    {
      QString text = "<b>" + signal->getPropertyValueString("name") + "</b>";
      if(auto* aspect = signal->getProperty("aspect")) /*[[likely]]*/
      {
        text.append("<br>" + aspect->displayName() + ": " + translateEnum(*aspect));
      }
      return text;
    }
  }
  else if(tileId == TileId::RailSensor)
  {
    if(auto sensor = m_board->getTileObject(l))
    {
      QString text = "<b>" + sensor->getPropertyValueString("name") + "</b>";
      if(auto* state = sensor->getProperty("state")) /*[[likely]]*/
      {
        text.append("<br>" + state->displayName() + ": " + translateEnum(*state));
      }
      return text;
    }
  }
  else if(tileId == TileId::RailBlock)
  {
    if(auto block = m_board->getTileObject(l))
    {
      QString text = "<b>" + block->getPropertyValueString("name") + "</b>";
      if(auto* state = block->getProperty("state")) /*[[likely]]*/
      {
        text.append("<br>" + state->displayName() + ": " + translateEnum(*state));
      }
      return text;
    }
  }
  else if(tileId == TileId::RailDirectionControl)
  {
    if(auto directionControl = m_board->getTileObject(l))
    {
      QString text = "<b>" + directionControl->getPropertyValueString("name") + "</b>";
      if(auto* state = directionControl->getProperty("state")) /*[[likely]]*/
      {
        text.append("<br>" + state->displayName() + ": " + translateEnum(*state));
      }
      return text;
    }
  }
  else if(tileId == TileId::RailSensor)
  {
    if(auto sensor = m_board->getTileObject(l))
    {
      QString text = "<b>" + sensor->getPropertyValueString("name") + "</b>";
      if(auto* state = sensor->getProperty("state")) /*[[likely]]*/
      {
        text.append("<br>" + state->displayName() + ": " + translateEnum(*state));
      }
      return text;
    }
  }
  else if(tileId == TileId::Switch)
  {
    if(auto switch_ = m_board->getTileObject(l))
    {
      QString text = "<b>" + switch_->getPropertyValueString("name") + "</b>";
      if(auto* value = switch_->getProperty("value")) /*[[likely]]*/
      {
        if(value->hasAttribute(AttributeName::AliasKeys)) /*[[likely]]*/
        {
          const QVariantList aliasKeys = value->getAttribute(AttributeName::AliasKeys, QVariant()).toList();
          const QVariantList aliasValues = value->getAttribute(AttributeName::AliasValues, QVariant()).toList();

          if(aliasKeys.size() == aliasValues.size()) /*[[likely]]*/
          {
            if(int index = aliasKeys.indexOf(value->toBool()); index != -1)
            {
              text.append("<br>" + value->displayName() + ": " + Locale::instance->parse(aliasValues[index].toString()));
            }
          }
        }
      }
      return text;
    }
  }
  else if(tileId == TileId::RailNXButton)
  {
    if(auto tile = m_board->getTileObject(l))
    {
      return "<b>" + tile->getPropertyValueString("name") + "</b>";
    }
  }
  return {};
}

bool BoardAreaWidget::event(QEvent* event)
{
  if(event->type() == QEvent::ToolTip)
  {
    QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
    const auto text = getTileToolTip(pointToTileLocation(helpEvent->pos()));
    if (!text.isEmpty())
    {
      QToolTip::showText(helpEvent->globalPos(), text);
    }
    else
    {
      QToolTip::hideText();
      event->ignore();
    }
    return true;
  }
  return QWidget::event(event);
}

void BoardAreaWidget::leaveEvent(QEvent* event)
{
  m_mouseMoveTileLocation = TileLocation::invalid;
  emit mouseTileLocationChanged(m_mouseMoveTileLocation.x, m_mouseMoveTileLocation.y);
  if(m_mouseMoveTileId != TileId::None)
    update();
  QWidget::leaveEvent(event);
}

void BoardAreaWidget::keyPressEvent(QKeyEvent* event)
{
  if(event->matches(QKeySequence::ZoomIn))
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
    if((event->pos() - m_mouseRightButtonPressedPoint).manhattanLength() < 5 || m_mouseMoveTileId != TileId::None)
      emit rightClicked();
  }
}

void BoardAreaWidget::mouseMoveEvent(QMouseEvent* event)
{
  if(hasMouseTracking())
  {
    const TileLocation tl = pointToTileLocation(event->pos());
    if(m_mouseMoveTileLocation != tl)
    {
      const TileLocation old = m_mouseMoveTileLocation;
      m_mouseMoveTileLocation = tl;
      emit mouseTileLocationChanged(tl.x, tl.y);
      if(m_mouseMoveAction != MouseMoveAction::None)
      {
        const int originX = boardLeft();
        const int originY = boardTop();
        const int tileSize = getTileSize();
        switch(m_mouseMoveAction)
        {
          case MouseMoveAction::AddTile:
          case MouseMoveAction::MoveTile:
            update(updateTileRect(old.x - originX, old.y - originY, m_mouseMoveTileWidth, m_mouseMoveTileHeight, tileSize));
            update(updateTileRect(tl.x - originX, tl.y - originY, m_mouseMoveTileWidth, m_mouseMoveTileHeight, tileSize));
            break;

          case MouseMoveAction::ResizeTile:
            update(updateTileRect(
              m_mouseMoveHideTileLocation.x - originX,
              m_mouseMoveHideTileLocation.y - originY,
              std::max(1, 1 + std::max(old.x, tl.x) - m_mouseMoveHideTileLocation.x),
              std::max(1, 1 + std::max(old.y, tl.y) - m_mouseMoveHideTileLocation.y),
              tileSize));
            break;

          case MouseMoveAction::None:
            break;
        }
      }
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
  assert(m_colorScheme);

  const QColor backgroundColor50{0x10, 0x10, 0x10, 0x80};
  const QColor backgroundColorError50{0xff, 0x00, 0x00, 0x80};
  const QColor gridColorHighlight{Qt::white};
  const QColor gridColorError{Qt::red};
  const int tileSize = getTileSize();
  const int gridSize = tileSize - 1;

  QPainter painter(this);

  const QRect viewport = rectToViewport(event->rect(), gridSize);

  painter.fillRect(viewport, m_colorScheme->background);

  // draw grid:
  switch(m_grid)
  {
    case BoardAreaGrid::None:
      break;

    case BoardAreaGrid::Line:
      painter.setPen(m_colorScheme->grid);
      for(int y = viewport.top(); y <= viewport.bottom(); y += gridSize)
        painter.drawLine(viewport.left(), y, viewport.right(), y);
      for(int x = viewport.left(); x <= viewport.right(); x += gridSize)
        painter.drawLine(x, viewport.top(), x, viewport.bottom());
      break;

    case BoardAreaGrid::Dot:
      painter.setPen(m_colorScheme->grid);
      for(int y = viewport.top(); y <= viewport.bottom(); y += gridSize)
        for(int x = viewport.left(); x <= viewport.right(); x += gridSize)
          painter.drawPoint(x, y);
      break;
  }

  painter.setRenderHint(QPainter::Antialiasing, true);

  // draw tiles:
  TilePainter tilePainter{painter, tileSize, *m_colorScheme};

  const int tileOriginX = boardLeft();
  const int tileOriginY = boardTop();
  const QRect tiles{tileOriginX + viewport.left() / gridSize, tileOriginY + viewport.top() / gridSize, viewport.width() / gridSize, viewport.height() / gridSize};

  painter.save();

  for(auto it : m_board->tileData())
    if(it.first.x + it.second.width() - 1 >= tiles.left() && it.first.x <= tiles.right() &&
        it.first.y + it.second.height() - 1 >= tiles.top() && it.first.y <= tiles.bottom())
    {
      if(it.first == m_mouseMoveHideTileLocation)
        continue;

      const TileId id = it.second.id();
      const TileRotate a = it.second.rotate();
      const uint8_t state = it.second.state;
      const bool isReserved = (state != 0);
      painter.setBrush(Qt::NoBrush);

      const QRectF r = drawTileRect(it.first.x - tileOriginX, it.first.y - tileOriginY, it.second.width(), it.second.height(), tileSize);
      switch(id)
      {
        case TileId::RailStraight:
        case TileId::RailCurve45:
        case TileId::RailCurve90:
        case TileId::RailBufferStop:
        case TileId::RailTunnel:
        case TileId::RailOneWay:
        case TileId::RailLink:
          tilePainter.draw(id, r, a, isReserved);
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
          tilePainter.drawTurnout(id, r, a, static_cast<TurnoutPosition>(state), getTurnoutPosition(it.first));
          break;

        case TileId::RailCross45:
        case TileId::RailCross90:
          tilePainter.drawCross(id, r, a, static_cast<CrossState>(state));
          break;

        case TileId::RailBridge45Left:
        case TileId::RailBridge45Right:
        case TileId::RailBridge90:
          tilePainter.drawBridge(id, r, a, state & 0x01, state & 0x02);
          break;

        case TileId::RailSensor:
          tilePainter.drawSensor(id, r, a, isReserved, getSensorState(it.first));
          break;

        case TileId::RailSignal2Aspect:
        case TileId::RailSignal3Aspect:
          tilePainter.drawSignal(id, r, a, isReserved, getSignalAspect(it.first));
          break;

        case TileId::RailBlock:
        {
          auto block = m_board->getTileObject(it.first);
          tilePainter.drawBlock(id, r, a, state & 0x01, state & 0x02, block);
          if(auto itColors = m_blockHighlight.blockColors().find(block->getPropertyValueString("id"));
              itColors != m_blockHighlight.blockColors().end() && !itColors->isEmpty())
          {
            for(int i = 0; i < itColors->size(); ++i)
            {
              QColor color = toQColor((*itColors)[i]);
              painter.setPen({});
              color.setAlphaF(m_colorScheme->blockHighlightAlpha);
              painter.setBrush(color);
              if(a == TileRotate::Deg0)
              {
                const auto h = r.height() / itColors->size();
                painter.drawRect(r.left(), r.top() + i * h, r.width(), h);
              }
              else
              {
                const auto w = r.width() / itColors->size();
                painter.drawRect(r.left() + i * w, r.top(), w, r.height());
              }
            }
          }
          break;
        }
        case TileId::RailDirectionControl:
          tilePainter.drawDirectionControl(id, r, a, isReserved, getDirectionControlState(it.first));
          break;

        case TileId::PushButton:
          if(auto button = m_board->getTileObject(it.first)) [[likely]]
          {
            tilePainter.drawPushButton(r,
              button->getPropertyValueEnum<Color>("color", Color::Yellow),
              button->getPropertyValueEnum<Color>("text_color", Color::Black),
              button->getPropertyValueString("text"));
          }
          else
          {
            tilePainter.drawPushButton(r);
          }
          break;

        case TileId::RailDecoupler:
          tilePainter.drawRailDecoupler(r, a, isReserved, getDecouplerState(it.first));
          break;

        case TileId::RailNXButton:
          tilePainter.drawRailNX(r, a, isReserved, getNXButtonEnabled(it.first), getNXButtonPressed(it.first));
          break;

        case TileId::Label:
        {
          if(auto label = m_board->getTileObject(it.first)) /*[[likely]]*/
          {
            tilePainter.drawLabel(r, a,
              label->getPropertyValueString("text"),
              label->getPropertyValueEnum<TextAlign>("text_align", TextAlign::Center),
              label->getPropertyValueEnum<Color>("text_color", Color::None),
              label->getPropertyValueEnum<Color>("background_color", Color::None));
          }
          else
          {
            tilePainter.drawLabel(r, a);
          }
          break;
        }
        case TileId::Switch:
          if(auto sw = m_board->getTileObject(it.first)) /*[[likely]]*/
          {
            tilePainter.drawSwitch(r,
              sw->getPropertyValueBool("value", false),
              sw->getPropertyValueEnum<Color>("color_on", Color::Yellow),
              sw->getPropertyValueEnum<Color>("color_off", Color::Gray));
          }
          else
          {
            tilePainter.drawSwitch(r);
          }
          break;

        case TileId::None:
        case TileId::ReservedForFutureExpension:
        default:
          assert(false);
          break;
      }
    }

  painter.restore();

  switch(m_mouseMoveAction)
  {
    case MouseMoveAction::AddTile:
    case MouseMoveAction::MoveTile:
    case MouseMoveAction::ResizeTile:
      if(m_mouseMoveTileId != TileId::None && m_mouseMoveTileLocation.isValid())
      {
        if(m_mouseMoveAction == MouseMoveAction::ResizeTile)
        {
          m_mouseMoveTileWidth = 1 + std::max(0, m_mouseMoveTileLocation.x - m_mouseMoveHideTileLocation.x);
          m_mouseMoveTileHeight = 1 + std::max(0, m_mouseMoveTileLocation.y - m_mouseMoveHideTileLocation.y);
        }

        const QRectF r =
          (m_mouseMoveAction == MouseMoveAction::ResizeTile)
            ? drawTileRect(
                m_mouseMoveHideTileLocation.x - tileOriginX,
                m_mouseMoveHideTileLocation.y - tileOriginY,
                m_mouseMoveTileWidth,
                m_mouseMoveTileHeight,
                tileSize)
            : drawTileRect(
                m_mouseMoveTileLocation.x - tileOriginX,
                m_mouseMoveTileLocation.y - tileOriginY,
                m_mouseMoveTileWidth,
                m_mouseMoveTileHeight,
                tileSize);

        if(m_mouseMoveAction == MouseMoveAction::ResizeTile &&
            (m_mouseMoveTileWidth > m_mouseMoveTileWidthMax || m_mouseMoveTileHeight > m_mouseMoveTileHeightMax))
        {
          painter.fillRect(r, backgroundColorError50);
          painter.setPen(gridColorError);
          painter.drawRect(r.adjusted(-0.5, -0.5, 0.5, 0.5));
        }
        else
        {
          painter.fillRect(r, backgroundColor50);
          painter.setPen(gridColorHighlight);
          painter.drawRect(r.adjusted(-0.5, -0.5, 0.5, 0.5));
          painter.setClipRect(r);
          tilePainter.draw(m_mouseMoveTileId, r, m_mouseMoveTileRotate);
        }
      }
      break;

    case MouseMoveAction::None:
      break;
  }
}

void BoardAreaWidget::dragEnterEvent(QDragEnterEvent *event)
{
  if(event->mimeData()->hasFormat(AssignTrainMimeData::mimeType))
  {
    m_dragMoveTileLocation = TileLocation::invalid;
    event->acceptProposedAction();
  }
}

void BoardAreaWidget::dragMoveEvent(QDragMoveEvent* event)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  const TileLocation l = pointToTileLocation(event->pos());
#else
  const TileLocation l = pointToTileLocation(event->position().toPoint());
#endif

  if(m_dragMoveTileLocation != l)
  {
    m_dragMoveTileLocation = l;
    if(event->mimeData()->hasFormat(AssignTrainMimeData::mimeType) &&
        m_board->getTileId(l) == TileId::RailBlock)
    {
      return event->accept();
    }
    event->ignore();
  }
}

void BoardAreaWidget::dropEvent(QDropEvent* event)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  const TileLocation l = pointToTileLocation(event->pos());
#else
  const TileLocation l = pointToTileLocation(event->position().toPoint());
#endif

  switch(m_board->getTileId(l))
  {
    case TileId::RailBlock:
      if(auto* assignTrain = dynamic_cast<const AssignTrainMimeData*>(event->mimeData()))
      {
        if(auto tile = std::dynamic_pointer_cast<BlockRailTile>(m_board->getTileObject(l)))
        {
          if(auto* method = tile->getMethod("assign_train"))
          {
            callMethod(*method, nullptr, assignTrain->trainId());
            return event->accept();
          }
        }
      }
      break;

    default:
      break;
  }
  event->ignore();
}

void BoardAreaWidget::settingsChanged()
{
  const auto& s = BoardSettings::instance();

  m_colorScheme = getBoardColorScheme(s.colorScheme.value());

  updateGrid();

  update();
}

void BoardAreaWidget::updateMinimumSize()
{
  const int tileSize = getTileSize() - 1;
  const int width = 1 + tileSize * (1 + boardRight() - boardLeft());
  const int height = 1 + tileSize * (1 + boardBottom() - boardTop());

  setMinimumSize(width, height);
  update();
}

void BoardAreaWidget::updateGrid()
{
  const bool editMode = m_board->connection()->world()->getPropertyValueBool(QStringLiteral("edit"), false);
  const auto& s = BoardSettings::instance();
  const auto grid = editMode ? s.gridEditMode.value() : s.gridOperateMode.value();

  if(m_grid != grid)
  {
    m_grid = grid;
    update();
  }
}
