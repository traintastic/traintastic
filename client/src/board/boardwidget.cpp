/**
 * client/src/board/boardwidget.cpp
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

#include "boardwidget.hpp"
#include <array>
#include <QVBoxLayout>
#include <QToolBar>
#include <QToolButton>
#include <QMenu>
#include <QAction>
#include <QScrollArea>
#include <QStatusBar>
#include <QLabel>
#include <QApplication>
#include <QKeyEvent>
#include <traintastic/locale/locale.hpp>
#include "../mainwindow.hpp"
#include "../network/board.hpp"
#include "../network/connection.hpp"
#include "../network/property.hpp"
#include "../network/method.hpp"
#include "../network/callmethod.hpp"
#include "../theme/theme.hpp"

struct TileInfo
{
  QString classId;
  TileId id;
  uint8_t rotates;
};

const std::array<TileInfo, 26> tileInfo = {
  TileInfo{QStringLiteral("board_tile.rail.straight"), TileId::RailStraight, 0xFF},
  TileInfo{QStringLiteral("board_tile.rail.buffer_stop"), TileId::RailBufferStop, 0xFF},
  TileInfo{QStringLiteral(""), TileId::None, 0},
  TileInfo{QStringLiteral("board_tile.rail.curve_45"), TileId::RailCurve45, 0xFF},
  TileInfo{QStringLiteral("board_tile.rail.curve_90"), TileId::RailCurve90, 0xFF},
  TileInfo{QStringLiteral(""), TileId::None, 0},
  TileInfo{QStringLiteral("board_tile.rail.cross_45"), TileId::RailCross45, 0x03},
  TileInfo{QStringLiteral("board_tile.rail.cross_90"), TileId::RailCross90, 0x03},
  TileInfo{QStringLiteral(""), TileId::None, 0},
  TileInfo{QStringLiteral("board_tile.rail.turnout_left_45"), TileId::RailTurnoutLeft45, 0xFF},
  TileInfo{QStringLiteral("board_tile.rail.turnout_left_90"), TileId::RailTurnoutLeft90, 0xFF},
  TileInfo{QStringLiteral("board_tile.rail.turnout_left_curved"), TileId::RailTurnoutLeftCurved, 0xFF},
  TileInfo{QStringLiteral("board_tile.rail.turnout_right_45"), TileId::RailTurnoutRight45, 0xFF},
  TileInfo{QStringLiteral("board_tile.rail.turnout_right_90"), TileId::RailTurnoutRight90, 0xFF},
  TileInfo{QStringLiteral("board_tile.rail.turnout_right_curved"), TileId::RailTurnoutRightCurved, 0xFF},
  TileInfo{QStringLiteral("board_tile.rail.turnout_wye"), TileId::RailTurnoutWye, 0xFF},
  TileInfo{QStringLiteral("board_tile.rail.turnout_3way"), TileId::RailTurnout3Way, 0xFF},
  TileInfo{QStringLiteral("board_tile.rail.turnout_singleslip"), TileId::RailTurnoutSingleSlip, 0xFF},
  TileInfo{QStringLiteral("board_tile.rail.turnout_doubleslip"), TileId::RailTurnoutDoubleSlip, 0xFF},
  TileInfo{QStringLiteral(""), TileId::None, 0},
  TileInfo{QStringLiteral("board_tile.rail.block"), TileId::RailBlock, 0x05},
  TileInfo{QStringLiteral("board_tile.rail.sensor"), TileId::RailSensor, 0xFF},
  TileInfo{QStringLiteral(""), TileId::None, 0},
  TileInfo{QStringLiteral("board_tile.rail.signal_2_aspect"), TileId::RailSignal2Aspect, 0xFF},
  TileInfo{QStringLiteral("board_tile.rail.signal_3_aspect"), TileId::RailSignal3Aspect, 0xFF},
  TileInfo{QStringLiteral(""), TileId::None, 0}
};

inline void validRotate(TileRotate& rotate, uint8_t rotates)
{
  Q_ASSERT(rotates != 0);
  while(((1 << static_cast<uint8_t>(rotate)) & rotates) == 0)
    rotate += TileRotate::Deg45;
}


BoardWidget::BoardWidget(std::shared_ptr<Board> object, QWidget* parent) :
  QWidget(parent),
  m_object{std::move(object)},
  m_boardArea{new BoardAreaWidget(*this, this)},
  m_statusBar{new QStatusBar(this)},
  m_statusBarMessage{new QLabel(this)},
  m_statusBarCoords{new QLabel(this)},
  m_editActions{new QActionGroup(this)},
  m_editRotate{TileRotate::Deg0}
  , m_tileMoveStarted{false}
{
  if(AbstractProperty* name = m_object->getProperty("name"))
  {
    connect(name, &AbstractProperty::valueChangedString, this, &BoardWidget::setWindowTitle);
    setWindowTitle(name->toString());
  }
  QMenu* m;

  QVBoxLayout* l = new QVBoxLayout();
  l->setMargin(0);

  // main toolbar:
  QToolBar* toolbar = new QToolBar(this);
  l->addWidget(toolbar);

  toolbar->addAction(/*Theme::getIcon("properties"),*/ Locale::tr("qtapp:board_properties"),
    []()
    {
      // TODO
    });

  toolbar->addSeparator();

  m_actionZoomIn = toolbar->addAction(Theme::getIcon("zoom_in"), Locale::tr("qtapp:zoom_in"), m_boardArea, &BoardAreaWidget::zoomIn);
  m_actionZoomOut = toolbar->addAction(Theme::getIcon("zoom_out"), Locale::tr("qtapp:zoom_out"), m_boardArea, &BoardAreaWidget::zoomOut);

  toolbar->addSeparator();

  m_toolButtonGrid = new QToolButton(this);
  m_toolButtonGrid->setIcon(Theme::getIcon("grid_dot"));
  m_toolButtonGrid->setToolTip(Locale::tr("qtapp:grid"));
  m_toolButtonGrid->setPopupMode(QToolButton::MenuButtonPopup);
  connect(m_toolButtonGrid, &QToolButton::pressed, m_toolButtonGrid, &QToolButton::showMenu);
  m = new QMenu(this);
  m_actionGridNone = m->addAction(Theme::getIcon("grid_none"), Locale::tr("qtapp:grid_none"),
    [this]()
    {
      m_boardArea->setGrid(BoardAreaWidget::Grid::None);
    });
  m_actionGridDot = m->addAction(Theme::getIcon("grid_dot"), Locale::tr("qtapp:grid_dot"),
    [this]()
    {
      m_boardArea->setGrid(BoardAreaWidget::Grid::Dot);
    });
  m_actionGridLine = m->addAction(Theme::getIcon("grid_line"), Locale::tr("qtapp:grid_line"),
    [this]()
    {
      m_boardArea->setGrid(BoardAreaWidget::Grid::Line);
    });
  m_toolButtonGrid->setMenu(m);
  toolbar->addWidget(m_toolButtonGrid);

  // edit toolbar:
  m_toolbarEdit = new QToolBar(this);
  l->addWidget(m_toolbarEdit);

  m_editActions->setExclusive(true);

  m_editActionNone = m_editActions->addAction(m_toolbarEdit->addAction(Theme::getIcon("mouse"), "", this,
    [this]()
    {
      actionSelected(nullptr);
    }));
  m_editActionNone->setCheckable(true);
  m_editActionNone->setData(-1);

  m_editActionMove = m_editActions->addAction(m_toolbarEdit->addAction(Theme::getIcon("move_tile"), Locale::tr("board:move_tile"), this,
    [this]()
    {
      actionSelected(nullptr);
    }));
  m_editActionMove->setCheckable(true);
  m_editActionMove->setData(-1);

  m_editActionDelete = m_editActions->addAction(m_toolbarEdit->addAction(Theme::getIcon("delete"), Locale::tr("board:delete_tile"), this,
    [this]()
    {
      actionSelected(nullptr);
    }));
  m_editActionDelete->setCheckable(true);
  m_editActionDelete->setData(-1);
  if(auto* m = m_object->getMethod("delete_tile"))
  {
    m_editActionDelete->setEnabled(m->getAttributeBool(AttributeName::Enabled, true));
    connect(m, &Method::attributeChanged, this,
      [this](AttributeName name, const QVariant& value)
      {
        if(name == AttributeName::Enabled)
        {
          m_editActionDelete->setEnabled(value.toBool());
          if(!m_editActionDelete->isEnabled() && m_editActionDelete->isChecked())
            m_editActionNone->setChecked(true);
        }
      });
  }
  else
    m_editActionDelete->setEnabled(false);

  m_toolbarEdit->addSeparator();
  {
    QVector<QAction*> actions;
    for(size_t i = 0; i < tileInfo.size(); i++)
    {
      const TileInfo& info = tileInfo[i];
      if(info.classId.isEmpty()) // next item group
      {
        if(actions.isEmpty())
          continue;
        else if(actions.length() == 1)
        {
          actions[0]->setCheckable(true);
          m_toolbarEdit->addAction(m_editActions->addAction(actions[0]));
          connect(actions[0], &QAction::triggered, this,
            [this, action=actions[0]]()
            {
              actionSelected(&tileInfo[action->data().toInt()]);
            });
          m_addActions.append(actions[0]);
        }
        else // > 1
        {
          QAction* action = m_editActions->addAction(m_toolbarEdit->addAction(""));
          m_addActions.append(action);
          if(auto* tb = dynamic_cast<QToolButton*>(m_toolbarEdit->widgetForAction(action)))
            tb->setPopupMode(QToolButton::MenuButtonPopup);
          QMenu* m = new QMenu(this);
          for(auto subAction : actions)
          {
            m->addAction(subAction);
            connect(subAction, &QAction::triggered, this,
              [this, action, subAction]()
              {
                action->setIcon(subAction->icon());
                action->setText(subAction->text());
                action->setData(subAction->data());
                action->setChecked(true);
                actionSelected(&tileInfo[subAction->data().toInt()]);
              });
          }
          action->setIcon(actions[0]->icon());
          action->setText(actions[0]->text());
          action->setData(actions[0]->data());
          action->setMenu(m);
          action->setCheckable(true);
          connect(action, &QAction::triggered, this,
            [this, action]()
            {
              actionSelected(&tileInfo[action->data().toInt()]);
            });
        }
        actions.clear();
      }
      else
      {
        QAction* act = new QAction(Theme::getIcon(info.classId), Locale::tr(QString("class_id:").append(info.classId)));
        act->setData(static_cast<qint64>(i));
        actions.append(act);
      }
    }

    if(auto* m = m_object->getMethod("add_tile"))
    {
      const bool v = m->getAttributeBool(AttributeName::Enabled, true);
      for(QAction* act : m_addActions)
        act->setEnabled(v);

      connect(m, &Method::attributeChanged, this,
        [this](AttributeName name, const QVariant& value)
        {
          if(name == AttributeName::Enabled)
          {
            const bool v = value.toBool();
            for(QAction* act : m_addActions)
            {
              act->setEnabled(v);
              if(!v && act->isChecked())
                m_editActionNone->setChecked(true);
            }
          }
        });
    }
    else
      for(QAction* act : m_addActions)
        act->setEnabled(false);
  }
  m_toolbarEdit->addSeparator();
  m_editActionResizeToContents = m_toolbarEdit->addAction(Theme::getIcon("resize_to_contents"), Locale::tr("board:resize_to_contents"), this,
    [this]()
    {
      if(Q_LIKELY(m_object))
        m_object->callMethod("resize_to_contents");
    });
  if(auto* m = m_object->getMethod("resize_to_contents"))
  {
    m_editActionResizeToContents->setEnabled(m->getAttributeBool(AttributeName::Enabled, true));
    connect(m, &Method::attributeChanged, this,
      [this](AttributeName name, const QVariant& value)
      {
        if(name == AttributeName::Enabled)
          m_editActionResizeToContents->setEnabled(value.toBool());
      });
  }
  else
    m_editActionResizeToContents->setEnabled(false);

  m_editActions->actions().first()->setChecked(true);

  QScrollArea* sa = new QScrollArea(this);
  sa->setWidgetResizable(true);
  sa->setWidget(m_boardArea);
  l->addWidget(sa);

  m_statusBar->addWidget(m_statusBarMessage, 1);
  m_statusBar->addWidget(m_statusBarCoords, 0);
  l->addWidget(m_statusBar);

  AbstractProperty* edit = m_object->connection()->world()->getProperty("edit");
  worldEditChanged(Q_LIKELY(edit) && edit->toBool());
  if(Q_LIKELY(edit))
    connect(edit, &AbstractProperty::valueChangedBool, this, &BoardWidget::worldEditChanged);

  setLayout(l);

  m_object->getTileData();

  connect(m_object.get(), &Board::tileDataChanged, this, [this](){ m_boardArea->update(); });
  connect(m_object.get(), &Board::tileObjectAdded, m_boardArea, &BoardAreaWidget::tileObjectAdded);
  connect(m_boardArea, &BoardAreaWidget::gridChanged, this, &BoardWidget::gridChanged);
  connect(m_boardArea, &BoardAreaWidget::zoomLevelChanged, this, &BoardWidget::zoomLevelChanged);
  connect(m_boardArea, &BoardAreaWidget::tileClicked, this, &BoardWidget::tileClicked);
  connect(m_boardArea, &BoardAreaWidget::rightClicked, this, &BoardWidget::rightClicked);
  connect(m_boardArea, &BoardAreaWidget::mouseTileLocationChanged, this,
    [this](int16_t x, int16_t y)
    {
      if(TileLocation{x, y}.isValid())
        m_statusBarCoords->setText(QString::number(x) + ", " + QString::number(y));
      else
        m_statusBarCoords->setText("");
    });

  gridChanged(m_boardArea->grid());
  zoomLevelChanged(m_boardArea->zoomLevel());
}

void BoardWidget::worldEditChanged(bool value)
{
  m_toolbarEdit->setVisible(value);
  m_statusBar->setVisible(value);
  m_boardArea->setMouseTracking(value);
}

void BoardWidget::gridChanged(BoardAreaWidget::Grid value)
{
  switch(value)
  {
    case BoardAreaWidget::Grid::None:
      m_toolButtonGrid->setIcon(m_actionGridNone->icon());
      break;

    case BoardAreaWidget::Grid::Dot:
      m_toolButtonGrid->setIcon(m_actionGridDot->icon());
      break;

    case BoardAreaWidget::Grid::Line:
      m_toolButtonGrid->setIcon(m_actionGridLine->icon());
      break;
  }
}

void BoardWidget::zoomLevelChanged(int value)
{
  m_actionZoomIn->setEnabled(value < BoardAreaWidget::zoomLevelMax);
  m_actionZoomOut->setEnabled(value > BoardAreaWidget::zoomLevelMin);
}

void BoardWidget::tileClicked(int16_t x, int16_t y)
{
  if(m_toolbarEdit->isVisible()) // edit mode
  {
    QAction* act = m_editActions->checkedAction();
    if(!act)
      return;

    if(act == m_editActionNone)
    {
      if(ObjectPtr obj = m_object->getTileObject({x, y}))
        MainWindow::instance->showObject(obj);
    }
    else if(act == m_editActionMove)
    {
      if(!m_tileMoveStarted) // grab
      {
        m_tileMoveX = x;
        m_tileMoveY = y;
        m_tileMoveStarted = true;
      }
      else // drop
      {
        m_object->moveTile(m_tileMoveX, m_tileMoveY, x, y, false,
          [this](const bool& r, Message::ErrorCode ec)
          {
          });
        m_tileMoveStarted = false;
      }
    }
    else if(act == m_editActionDelete)
    {
      m_object->deleteTile(x, y,
        [this](const bool& r, Message::ErrorCode ec)
        {
        });
    }
    else // add
    {
      const QString& classId = tileInfo[act->data().toInt()].classId;
      const Qt::KeyboardModifiers kbMod = QApplication::keyboardModifiers();
      if(kbMod == Qt::NoModifier || kbMod == Qt::ControlModifier)
        m_object->addTile(x, y, m_editRotate, classId, kbMod == Qt::ControlModifier,
          [this](const bool& r, Message::ErrorCode ec)
          {
          });
    }
  }
  else
  {
    auto it = m_object->tileData().find({x, y});
    if(it != m_object->tileData().end())
    {
      if(ObjectPtr obj = m_object->getTileObject({x, y}))
      {
        if(isRailTurnout(it->second.id()))
        {
          if(auto* m = obj->getMethod("next_position"))
            callMethod(*m, nullptr, false);
        }
        else if(isRailSignal(it->second.id()))
        {
          if(auto* m = obj->getMethod("next_aspect"))
            callMethod(*m, nullptr, false);
        }
      }
    }
  }
}

void BoardWidget::rightClicked()
{
  if(QAction* act = m_editActions->checkedAction())
    if(int index = act->data().toInt(); index >= 0 && Q_LIKELY(index < tileInfo.size()))
    {
      m_editRotate += TileRotate::Deg45;
      validRotate(m_editRotate, tileInfo[index].rotates);
      m_boardArea->setMouseMoveTileRotate(m_editRotate);
    }
}

void BoardWidget::actionSelected(const TileInfo* tileInfo)
{
  m_tileMoveStarted = false;

  if(tileInfo)
  {
    validRotate(m_editRotate, tileInfo->rotates);
    m_boardArea->setMouseMoveTileRotate(m_editRotate);
    m_boardArea->setMouseMoveTileId(tileInfo->id);
  }
  else
    m_boardArea->setMouseMoveTileId(TileId::None);
}

void BoardWidget::keyPressEvent(QKeyEvent* event)
{
  if(event->key() == Qt::Key_Escape && m_tileMoveStarted)
    m_tileMoveStarted = false;
  else
    QWidget::keyPressEvent(event);
}
