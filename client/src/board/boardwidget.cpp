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
#include <traintastic/locale/locale.hpp>
#include "../mainwindow.hpp"
#include "../network/board.hpp"
#include "../network/connection.hpp"
#include "../network/property.hpp"
#include "../network/callmethod.hpp"

struct TileInfo
{
  QString classId;
  uint8_t rotates;
};

const std::array<TileInfo, 26> tileInfo = {
  TileInfo{QStringLiteral("board_tile.rail.straight"), 0xFF},
  TileInfo{QStringLiteral("board_tile.rail.buffer_stop"), 0xFF},
  TileInfo{QStringLiteral(""), 0},
  TileInfo{QStringLiteral("board_tile.rail.curve_45"), 0xFF},
  TileInfo{QStringLiteral("board_tile.rail.curve_90"), 0xFF},
  TileInfo{QStringLiteral(""), 0},
  TileInfo{QStringLiteral("board_tile.rail.cross_45"), 0x03},
  TileInfo{QStringLiteral("board_tile.rail.cross_90"), 0x03},
  TileInfo{QStringLiteral(""), 0},
  TileInfo{QStringLiteral("board_tile.rail.turnout_left_45"), 0xFF},
  TileInfo{QStringLiteral("board_tile.rail.turnout_left_90"), 0xFF},
  TileInfo{QStringLiteral("board_tile.rail.turnout_left_curved"), 0xFF},
  TileInfo{QStringLiteral("board_tile.rail.turnout_right_45"), 0xFF},
  TileInfo{QStringLiteral("board_tile.rail.turnout_right_90"), 0xFF},
  TileInfo{QStringLiteral("board_tile.rail.turnout_right_curved"), 0xFF},
  TileInfo{QStringLiteral("board_tile.rail.turnout_wye"), 0xFF},
  TileInfo{QStringLiteral("board_tile.rail.turnout_3way"), 0xFF},
  TileInfo{QStringLiteral("board_tile.rail.turnout_singleslip"), 0xFF},
  TileInfo{QStringLiteral("board_tile.rail.turnout_doubleslip"), 0xFF},
  TileInfo{QStringLiteral(""), 0},
  TileInfo{QStringLiteral("board_tile.rail.block"), 0x05},
  TileInfo{QStringLiteral("board_tile.rail.sensor"), 0xFF},
  TileInfo{QStringLiteral(""), 0},
  TileInfo{QStringLiteral("board_tile.rail.signal_2_aspect"), 0xFF},
  TileInfo{QStringLiteral("board_tile.rail.signal_3_aspect"), 0xFF},
  TileInfo{QStringLiteral(""), 0}
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

  toolbar->addAction(/*QIcon(":/dark/properties.svg"),*/ Locale::tr("qtapp:board_properties"),
    []()
    {
      // TODO
    });

  toolbar->addSeparator();

  m_actionZoomIn = toolbar->addAction(QIcon(":/dark/zoom_in.svg"), Locale::tr("qtapp:zoom_in"), m_boardArea, &BoardAreaWidget::zoomIn);
  m_actionZoomOut = toolbar->addAction(QIcon(":/dark/zoom_out.svg"), Locale::tr("qtapp:zoom_out"), m_boardArea, &BoardAreaWidget::zoomOut);

  toolbar->addSeparator();

  m_toolButtonGrid = new QToolButton(this);
  m_toolButtonGrid->setIcon(QIcon(":/dark/grid_dot.svg"));
  m_toolButtonGrid->setToolTip(Locale::tr("qtapp:grid"));
  m_toolButtonGrid->setPopupMode(QToolButton::MenuButtonPopup);
  connect(m_toolButtonGrid, &QToolButton::pressed, m_toolButtonGrid, &QToolButton::showMenu);
  m = new QMenu(this);
  m_actionGridNone = m->addAction(QIcon(":/dark/grid_none.svg"), Locale::tr("qtapp:grid_none"),
    [this]()
    {
      m_boardArea->setGrid(BoardAreaWidget::Grid::None);
    });
  m_actionGridDot = m->addAction(QIcon(":/dark/grid_dot.svg"), Locale::tr("qtapp:grid_dot"),
    [this]()
    {
      m_boardArea->setGrid(BoardAreaWidget::Grid::Dot);
    });
  m_actionGridLine = m->addAction(QIcon(":/dark/grid_line.svg"), Locale::tr("qtapp:grid_line"),
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

  m_editActionNone = m_editActions->addAction(m_toolbarEdit->addAction(QIcon(":/dark/mouse.svg"), ""));
  m_editActionNone->setCheckable(true);
  m_editActionNone->setData(-1);

  m_editActionMove = m_editActions->addAction(m_toolbarEdit->addAction(QIcon(":/dark/move_tile.svg"), Locale::tr("board:move_tile")));
  m_editActionMove->setCheckable(true);
  m_editActionMove->setData(-1);
  m_editActionMove->setEnabled(false); // todo: implement

  m_editActionDelete = m_editActions->addAction(m_toolbarEdit->addAction(QIcon(":/dark/delete.svg"), Locale::tr("board:delete_tile")));
  m_editActionDelete->setCheckable(true);
  m_editActionDelete->setData(-1);

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
              validRotate(m_editRotate, tileInfo[action->data().toInt()].rotates);
            });
        }
        else // > 1
        {
          QAction* action = m_editActions->addAction(m_toolbarEdit->addAction(""));
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
                validRotate(m_editRotate, tileInfo[subAction->data().toInt()].rotates);
              });
          }
          action->setIcon(actions[0]->icon());
          action->setText(actions[0]->text());
          action->setData(actions[0]->data());
          action->setMenu(m);
          action->setCheckable(true);
        }
        actions.clear();
      }
      else
      {
        QAction* act = new QAction(QIcon(QString(":/dark/").append(info.classId).append(".svg")), Locale::tr(QString("class_id:").append(info.classId)));
        act->setData(static_cast<qint64>(i));
        actions.append(act);
      }
    }
  }

  m_editActions->actions().first()->setChecked(true);

  QScrollArea* sa = new QScrollArea(this);
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
  connect(m_boardArea, &BoardAreaWidget::gridChanged, this, &BoardWidget::gridChanged);
  connect(m_boardArea, &BoardAreaWidget::zoomLevelChanged, this, &BoardWidget::zoomLevelChanged);
  connect(m_boardArea, &BoardAreaWidget::tileClicked, this, &BoardWidget::tileClicked);
  connect(m_boardArea, &BoardAreaWidget::rightClicked, this, &BoardWidget::rightClicked);
  connect(m_boardArea, &BoardAreaWidget::mouseTileLocationChanged, this,
    [this](int16_t x, int16_t y)
    {
      m_statusBarCoords->setText(QString::number(x) + ", " + QString::number(y));
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
      auto it = m_object->tileData().find({x, y});
      if(it != m_object->tileData().end())
        if(ObjectPtr obj = m_object->getTileObject({x, y}))
          MainWindow::instance->showObject(obj);
    }
    else if(act == m_editActionMove)
    {
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
    }
}
