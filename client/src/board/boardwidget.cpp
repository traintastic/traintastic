/**
 * client/src/board/boardwidget.cpp
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

#include "boardwidget.hpp"
#include <array>
#include <QVBoxLayout>
#include <QToolBar>
#include <QToolButton>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QScrollArea>
#include <QStatusBar>
#include <QLabel>
#include <QApplication>
#include <QKeyEvent>
#include <QPainter>
#include <traintastic/locale/locale.hpp>
#include "getboardcolorscheme.hpp"
#include "tilepainter.hpp"
#include "tilemenu.hpp"
#include "../mainwindow.hpp"
#include "../network/connection.hpp"
#include "../network/property.hpp"
#include "../network/method.hpp"
#include "../network/callmethod.hpp"
#include "../network/object/nxbuttonrailtile.hpp"
#include "../theme/theme.hpp"
#include "../utils/enum.hpp"
#include "../utils/trysetlocalename.hpp"
#include "../settings/boardsettings.hpp"
#include <traintastic/utils/clamp.hpp>

inline TileRotate rotateCW(TileRotate rotate, uint8_t rotates)
{
  assert(rotates != 0);
  do
  {
    rotate += TileRotate::Deg45;
  }
  while(((1 << static_cast<uint8_t>(rotate)) & rotates) == 0);
  return rotate;
}

inline TileRotate rotateCCW(TileRotate rotate, uint8_t rotates)
{
  assert(rotates != 0);
  do
  {
    rotate -= TileRotate::Deg45;
  }
  while(((1 << static_cast<uint8_t>(rotate)) & rotates) == 0);
  return rotate;
}

inline void validRotate(TileRotate& rotate, uint8_t rotates)
{
  Q_ASSERT(rotates != 0);
  while(((1 << static_cast<uint8_t>(rotate)) & rotates) == 0)
    rotate += TileRotate::Deg45;
}


BoardWidget::BoardWidget(std::shared_ptr<Board> object, QWidget* parent) :
  QWidget(parent),
  m_object{std::move(object)},
  m_nxManagerRequestId{Connection::invalidRequestId},
  m_boardArea{new BoardAreaWidget(m_object, this)},
  m_statusBar{new QStatusBar(this)},
  m_statusBarMessage{new QLabel(this)},
  m_statusBarCoords{new QLabel(this)},
  m_statusBarZoom{new QLabel(this)},
  m_editActions{new QActionGroup(this)}
  , m_tileMoveStarted{false}
  , m_tileResizeStarted{false}
  , m_nxButtonTimerId(0)
{
  Theme::setWindowIcon(*this, object->classId);
  setFocusPolicy(Qt::StrongFocus);

  if(AbstractProperty* name = m_object->getProperty("name"))
  {
    connect(name, &AbstractProperty::valueChangedString, this, &BoardWidget::setWindowTitle);
    setWindowTitle(name->toString());
  }

  QVBoxLayout* l = new QVBoxLayout();
  l->setContentsMargins(0, 0, 0, 0);

  // main toolbar:
  QToolBar* toolbar = new QToolBar(this);
  l->addWidget(toolbar);

  toolbar->addAction(Theme::getIcon("edit"), Locale::tr("qtapp:board_properties"),
    [this]()
    {
      MainWindow::instance->showObject(m_object, SubWindowType::Object);
    });

  toolbar->addSeparator();

  m_actionZoomIn = toolbar->addAction(Theme::getIcon("zoom_in"), Locale::tr("qtapp:zoom_in"), m_boardArea, &BoardAreaWidget::zoomIn);
  m_actionZoomOut = toolbar->addAction(Theme::getIcon("zoom_out"), Locale::tr("qtapp:zoom_out"), m_boardArea, &BoardAreaWidget::zoomOut);

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
  if(auto* method = m_object->getMethod("move_tile"))
  {
    m_editActionMove->setEnabled(method->getAttributeBool(AttributeName::Enabled, true));
    connect(method, &Method::attributeChanged, this,
      [this](AttributeName name, const QVariant& value)
      {
        if(name == AttributeName::Enabled)
        {
          m_editActionMove->setEnabled(value.toBool());
          if(!m_editActionMove->isEnabled() && m_editActionMove->isChecked())
            m_editActionNone->activate(QAction::Trigger);
        }
      });
  }
  else
    m_editActionMove->setEnabled(false);

  m_editActionResize = m_editActions->addAction(m_toolbarEdit->addAction(Theme::getIcon("resize_tile"), Locale::tr("board:resize_tile"), this,
    [this]()
    {
      actionSelected(nullptr);
    }));
  m_editActionResize->setCheckable(true);
  m_editActionResize->setData(-1);
  if(auto* method = m_object->getMethod("resize_tile"))
  {
    m_editActionResize->setEnabled(method->getAttributeBool(AttributeName::Enabled, true));
    connect(method, &Method::attributeChanged, this,
      [this](AttributeName name, const QVariant& value)
      {
        if(name == AttributeName::Enabled)
        {
          m_editActionResize->setEnabled(value.toBool());
          if(!m_editActionResize->isEnabled() && m_editActionResize->isChecked())
            m_editActionNone->activate(QAction::Trigger);
        }
      });
  }
  else
    m_editActionResize->setEnabled(false);

  m_editActionDelete = m_editActions->addAction(m_toolbarEdit->addAction(Theme::getIcon("delete"), Locale::tr("board:delete_tile"), this,
    [this]()
    {
      actionSelected(nullptr);
    }));
  m_editActionDelete->setCheckable(true);
  m_editActionDelete->setData(-1);
  if(auto* method = m_object->getMethod("delete_tile"))
  {
    m_editActionDelete->setEnabled(method->getAttributeBool(AttributeName::Enabled, true));
    connect(method, &Method::attributeChanged, this,
      [this](AttributeName name, const QVariant& value)
      {
        if(name == AttributeName::Enabled)
        {
          m_editActionDelete->setEnabled(value.toBool());
          if(!m_editActionDelete->isEnabled() && m_editActionDelete->isChecked())
            m_editActionNone->activate(QAction::Trigger);
        }
      });
  }
  else
    m_editActionDelete->setEnabled(false);

  m_toolbarEdit->addSeparator();
  {
    auto addItems =
      [this](const QVector<QAction*>& actions)
      {
        if(actions.length() == 1)
        {
          actions[0]->setCheckable(true);
          m_toolbarEdit->addAction(m_editActions->addAction(actions[0]));
          connect(actions[0], &QAction::triggered, this,
            [this, action=actions[0]]()
            {
              actionSelected(&Board::tileInfo[action->data().toInt()]);
            });
          m_addActions.append(actions[0]);
        }
        else if(actions.length() > 1)
        {
          QAction* action = m_editActions->addAction(m_toolbarEdit->addAction(""));
          m_addActions.append(action);
          if(auto* tb = dynamic_cast<QToolButton*>(m_toolbarEdit->widgetForAction(action)))
            tb->setPopupMode(QToolButton::MenuButtonPopup);
          QMenu* toolbuttonMenu = new QMenu(this);
          for(auto subAction : actions)
          {
            toolbuttonMenu->addAction(subAction);
            connect(subAction, &QAction::triggered, this,
              [this, action, subAction]()
              {
                action->setIcon(subAction->icon());
                action->setText(subAction->text());
                action->setData(subAction->data());
                action->setChecked(true);
                actionSelected(&Board::tileInfo[subAction->data().toInt()]);
              });
          }
          action->setIcon(actions[0]->icon());
          action->setText(actions[0]->text());
          action->setData(actions[0]->data());
          action->setMenu(toolbuttonMenu);
          action->setCheckable(true);
          connect(action, &QAction::triggered, this,
            [this, action]()
            {
              actionSelected(&Board::tileInfo[action->data().toInt()]);
            });
        }
      };

    //! \todo add multi level menu support (not yet used)
    QStringList lastMenu;
    QVector<QAction*> actions;
    for(size_t i = 0; i < Board::tileInfo.size(); i++)
    {
      const auto& info = Board::tileInfo[i];
      if(info.menu != lastMenu)
      {
        lastMenu = info.menu;
        addItems(actions);
        actions.clear();
      }

      QAction* act = new QAction(Theme::getIcon(info.classId), Locale::tr(QString("class_id:").append(info.classId)));
      act->setData(static_cast<qint64>(i));
      actions.append(act);
    }
    addItems(actions);

    if(auto* method = m_object->getMethod("add_tile"))
    {
      {
        const bool v = method->getAttributeBool(AttributeName::Enabled, true);
        for(QAction* act : m_addActions)
          act->setEnabled(v);
      }

      connect(method, &Method::attributeChanged, this,
        [this](AttributeName name, const QVariant& value)
        {
          if(name == AttributeName::Enabled)
          {
            const bool v = value.toBool();
            for(QAction* act : m_addActions)
            {
              act->setEnabled(v);
              if(!v && act->isChecked())
                m_editActionNone->activate(QAction::Trigger);
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
  if(auto* method = m_object->getMethod("resize_to_contents"))
  {
    m_editActionResizeToContents->setEnabled(method->getAttributeBool(AttributeName::Enabled, true));
    connect(method, &Method::attributeChanged, this,
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
  m_statusBar->addWidget(m_statusBarZoom, 0);
  l->addWidget(m_statusBar);

  AbstractProperty* edit = m_object->connection()->world()->getProperty("edit");
  worldEditChanged(Q_LIKELY(edit) && edit->toBool());
  if(Q_LIKELY(edit))
    connect(edit, &AbstractProperty::valueChangedBool, this, &BoardWidget::worldEditChanged);

  setLayout(l);

  m_object->getTileData();

  m_nxManagerRequestId = m_object->connection()->getObject("world.nx_manager",
    [this](const ObjectPtr& nxManager, std::optional<const Error> /*error*/)
    {
      m_nxManagerRequestId = Connection::invalidRequestId;

      if(nxManager)
      {
        m_nxManager = nxManager;
      }
    });

  connect(m_object.get(), &Board::tileDataChanged, this, [this](){ m_boardArea->update(); });
  connect(m_object.get(), &Board::tileObjectAdded, m_boardArea, &BoardAreaWidget::tileObjectAdded);
  connect(m_boardArea, &BoardAreaWidget::zoomLevelChanged, this, &BoardWidget::zoomLevelChanged);
  connect(m_boardArea, &BoardAreaWidget::tileClicked, this, &BoardWidget::tileClicked);
  connect(m_boardArea, &BoardAreaWidget::rightClicked, this, &BoardWidget::rightClicked);
  connect(m_boardArea, &BoardAreaWidget::mouseTileLocationChanged, this,
    [this](int16_t x, int16_t y)
    {
      if(const auto tl = TileLocation{x, y}; tl.isValid())
      {
        m_statusBarCoords->setText(QString::number(x) + ", " + QString::number(y));

        const auto tileId = m_object->getTileId(tl);
        auto cursorShape = Qt::ArrowCursor;

        if(m_toolbarEdit->isVisible()) // Edit mode
        {
          if(isActive(tileId) && m_editActions->checkedAction() == m_editActionNone)
          {
            cursorShape = Qt::PointingHandCursor;
          }
        }
        else // Operate mode
        {
          if(isRailTurnout(tileId) ||
              isRailSignal(tileId) ||
              tileId == TileId::RailDirectionControl ||
              tileId == TileId::RailDecoupler ||
              tileId == TileId::PushButton ||
              tileId == TileId::Switch)
          {
            cursorShape = Qt::PointingHandCursor;
          }
          else if(tileId == TileId::RailNXButton)
          {
            if(auto nxButton = m_object->getTileObject(tl); nxButton && nxButton->getPropertyValueBool("enabled", false))
            {
              cursorShape = Qt::PointingHandCursor;
            }
          }
          else if(tileId == TileId::RailSensor)
          {
            if(auto sensor = m_object->getTileObject(tl))
            {
              if(auto* simulateTrigger = sensor->getMethod("simulate_trigger");
                  simulateTrigger && simulateTrigger->getAttributeBool(AttributeName::Enabled, false))
              {
                cursorShape = Qt::PointingHandCursor;
              }
            }
          }
        }

        setCursor(cursorShape);
      }
      else
        m_statusBarCoords->setText("");
    });

  m_boardArea->setMouseTracking(true);
  zoomLevelChanged(m_boardArea->zoomLevel());
}

BoardWidget::~BoardWidget()
{
  stopTimerAndReleaseButtons();

  if(m_nxManagerRequestId != Connection::invalidRequestId)
  {
    m_object->connection()->cancelRequest(m_nxManagerRequestId);
  }
}

void BoardWidget::worldEditChanged(bool value)
{
  if(!value)
    m_editActionNone->activate(QAction::Trigger);
  m_toolbarEdit->setVisible(value);
  m_statusBar->setVisible(value);

  m_boardArea->updateGrid();

  // Stop timers in edit mode
  if(value)
    stopTimerAndReleaseButtons();
}

void BoardWidget::zoomLevelChanged(int value)
{
  m_actionZoomIn->setEnabled(value < BoardAreaWidget::zoomLevelMax);
  m_actionZoomOut->setEnabled(value > BoardAreaWidget::zoomLevelMin);
  m_statusBarZoom->setText(QString::number(100 * m_boardArea->zoomRatio(), 'f', 0) + " %");
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
        TileLocation l{x, y};
        if(m_object->getTileOrigin(l))
        {
          m_tileMoveX = x;
          m_tileMoveY = y;
          m_tileMoveStarted = true;

          const auto& tileData = m_object->tileData().at(l);
          m_boardArea->setMouseMoveAction(BoardAreaWidget::MouseMoveAction::MoveTile);
          m_boardArea->setMouseMoveTileId(tileData.id());
          m_boardArea->setMouseMoveTileRotate(tileData.rotate());
          m_boardArea->setMouseMoveTileSize(tileData.width(), tileData.height());
          m_boardArea->setMouseMoveHideTileLocation(l);
          m_tileRotateLast = tileData.rotate();

          if(auto it = std::find_if(Board::tileInfo.begin(), Board::tileInfo.end(), [id=tileData.id()](const auto& v){ return v.tileId == id; }); it != Board::tileInfo.end())
            m_tileRotates = it->rotates;
        }
      }
      else // drop
      {
        m_object->moveTile(m_tileMoveX, m_tileMoveY, x, y, m_boardArea->mouseMoveTileRotate(), false,
          [](const bool& /*r*/, std::optional<const Error> /*error*/)
          {
          });
        m_tileMoveStarted = false;
        m_boardArea->setMouseMoveAction(BoardAreaWidget::MouseMoveAction::None);
      }
    }
    else if(act == m_editActionResize)
    {
      if(!m_tileResizeStarted) // start
      {
        TileLocation l{x, y};
        if(m_object->getTileOrigin(l))
        {
          const auto tile = m_object->getTileObject(l);
          if(!tile)
            return; // no resize possible

          const auto* width = tile->getProperty("width");
          const auto* height = tile->getProperty("height");
          const uint8_t widthMax = width ? clamp<uint8_t>(width->getAttributeInt64(AttributeName::Max, 1)) : 1;
          const uint8_t heightMax = height ? clamp<uint8_t>(height->getAttributeInt64(AttributeName::Max, 1)) : 1;

          if(widthMax <= 1 && heightMax <= 1)
            return; // no resize possible

          m_tileResizeX = l.x;
          m_tileResizeY = l.y;
          m_tileResizeStarted = true;

          const auto& tileData = m_object->tileData().at(l);
          m_boardArea->setMouseMoveAction(BoardAreaWidget::MouseMoveAction::ResizeTile);
          m_boardArea->setMouseMoveTileId(tileData.id());
          m_boardArea->setMouseMoveTileRotate(tileData.rotate());
          m_boardArea->setMouseMoveHideTileLocation(l);
          m_boardArea->setMouseMoveTileSizeMax(widthMax, heightMax);
        }
      }
      else // stop
      {
        const int16_t w = 1 + x - m_tileResizeX;
        const int16_t h = 1 + y - m_tileResizeY;
        if(w >= 1 && w <= std::numeric_limits<uint8_t>::max() &&
            h >= 1 && h <= std::numeric_limits<uint8_t>::max())
        {
          m_object->resizeTile(m_tileResizeX, m_tileResizeY, w, h,
            [](const bool& /*r*/, std::optional<const Error> /*error*/)
            {
            });

          m_tileResizeStarted = false;
          m_boardArea->setMouseMoveAction(BoardAreaWidget::MouseMoveAction::None);
        }
      }
    }
    else if(act == m_editActionDelete)
    {
      m_object->deleteTile(x, y,
        [](const bool& /*r*/, std::optional<const Error> /*error*/)
        {
        });
    }
    else // add
    {
      const QString& classId = Board::tileInfo[act->data().toInt()].classId;
      const Qt::KeyboardModifiers kbMod = QApplication::keyboardModifiers();
      if(kbMod == Qt::NoModifier || kbMod == Qt::ControlModifier)
        m_object->addTile(x, y, m_boardArea->mouseMoveTileRotate(), classId, kbMod == Qt::ControlModifier,
          [this, x, y](const bool& r, std::optional<const Error> /*error*/)
          {
            if(r)
            {
              if(auto object = m_object->getTileObject({x, y}))
              {
                trySetLocaleName(*object);
              }
            }
          });
    }
  }
  else
  {
    if(ObjectPtr obj = m_object->getTileObject({x, y}))
    {
      const auto tileId = m_object->getTileId({x, y});

      if(tileId == TileId::PushButton)
      {
        if(auto* m = obj->getMethod("pressed"))
          m->call();
      }
      else if(tileId == TileId::Switch)
      {
        if(auto* value = obj->getProperty("value")) /*[[likely]]*/
        {
          if(auto* setValue = obj->getMethod("set_value")) /*[[likely]]*/
          {
            callMethod(*setValue, nullptr, !value->toBool());
          }
        }
      }
      else if(tileId == TileId::RailDecoupler)
      {
        if(const auto* state = obj->getProperty("state"))
        {
          switch(state->toEnum<DecouplerState>())
          {
            case DecouplerState::Deactivated:
              obj->callMethod("activate");
              break;

            case DecouplerState::Activated:
              obj->callMethod("deactivate");
              break;
          }
        }
      }
      else if(tileId == TileId::RailBlock)
      {
        TileMenu::getBlockRailTileMenu(obj, this)->exec(QCursor::pos());
      }
      else if(tileId == TileId::RailNXButton)
      {
        if(auto nxButton = std::dynamic_pointer_cast<NXButtonRailTile>(obj)) /*[[likely]]*/
        {
          if(!nxButton->getPropertyValueBool("enabled", false))
          {
            return; // not enabled, no action
          }

          if(nxButton->isPressed())
          {
            stopTimerAndReleaseButtons();
            return;
          }
          else
          {
            nxButton->setPressed(true);
          }

          if(auto firstButton = m_nxButtonPressed.lock())
          {
            if(m_nxManager) /*[[likely]]*/
            {
              if(auto* method = m_nxManager->getMethod("select")) /*[[likely]]*/
              {
                callMethod(*method, nullptr, firstButton, nxButton);
              }
            }

            m_nxButtonPressed.reset();

            startReleaseTimer(firstButton, nxButton);
          }
          else
          {
            m_nxButtonPressed = nxButton;

            startHoldTimer(nxButton);
          }
        }
      }
      else if(tileId == TileId::RailSensor)
      {
        obj->callMethod("simulate_trigger");
      }
      else
      {
        AbstractProperty* value = nullptr;
        Method* setValue = nullptr;
        if(isRailTurnout(tileId))
        {
          value = obj->getProperty("position");
          setValue = obj->getMethod("set_position");
        }
        else if(isRailSignal(tileId))
        {
          value = obj->getProperty("aspect");
          setValue = obj->getMethod("set_aspect");
        }
        else if(tileId == TileId::RailDirectionControl)
        {
          value = obj->getProperty("state");
          setValue = obj->getMethod("set_state");
        }

        if(value && setValue)
        {
          const auto values = setValue->getAttribute(AttributeName::Values, QVariant()).toList();

          if(values.size() == 2)
          {
            const auto n = (value->toInt() == values[0].toInt()) ? values[1].toInt() : values[0].toInt();
            callMethod(*setValue, nullptr, n);
          }
          else if(values.size() > 2)
          {
            auto tileRotate = TileRotate::Deg0;
            if(auto* p = obj->getProperty("rotate"))
              tileRotate = p->toEnum<TileRotate>();

            const int iconSize = 16;
            QImage image(iconSize, iconSize, QImage::Format_ARGB32);
            QPainter painter{&image};
            painter.setRenderHint(QPainter::Antialiasing, true);
            TilePainter tilePainter{painter, iconSize, *getBoardColorScheme(BoardSettings::instance().colorScheme.value())};

            QMenu menu(this);
            for(const auto& v : values)
            {
              const auto n = v.toInt();

              image.fill(Qt::transparent);

              if(isRailTurnout(tileId))
                tilePainter.drawTurnout(tileId, image.rect(), tileRotate, TurnoutPosition::Unknown, static_cast<TurnoutPosition>(n));
              else if(isRailSignal(tileId))
                tilePainter.drawSignal(tileId, image.rect(), tileRotate, false, static_cast<SignalAspect>(n));
              else if(tileId == TileId::RailDirectionControl)
                tilePainter.drawDirectionControl(tileId, image.rect(), tileRotate, false, static_cast<DirectionControlState>(n));

              connect(menu.addAction(QIcon(QPixmap::fromImage(image)), translateEnum(value->enumName(), n)), &QAction::triggered,
                [setValue, n]()
                {
                  callMethod(*setValue, nullptr, n);
                });
            }
            menu.exec(QCursor::pos());
          }
        }
      }
    }
  }
}

void BoardWidget::rightClicked()
{
  if(QApplication::keyboardModifiers() == Qt::NoModifier)
    rotateTile();
  else if(QApplication::keyboardModifiers() == Qt::ShiftModifier)
    rotateTile(true);
}

void BoardWidget::startHoldTimer(const std::shared_ptr<NXButtonRailTile>& nxButton)
{
  stopTimerAndReleaseButtons();

  m_releaseButton1 = nxButton;

  assert(m_nxButtonTimerId == 0);
  m_nxButtonTimerId = startTimer(nxButtonHoldTime);
}

void BoardWidget::startReleaseTimer(const std::shared_ptr<NXButtonRailTile> &firstButton,
                                     const std::shared_ptr<NXButtonRailTile> &nxButton)
{
  // Do not release first button yet
  m_releaseButton1.reset();

  stopTimerAndReleaseButtons();

  m_releaseButton1 = firstButton;
  m_releaseButton2 = nxButton;

  assert(m_nxButtonTimerId == 0);
  m_nxButtonTimerId = startTimer(nxButtonReleaseDelay);
}

void BoardWidget::stopTimerAndReleaseButtons()
{
  if(m_nxButtonTimerId)
  {
    // Instantly release buttons
    if(auto btn = m_releaseButton1.lock())
    {
        releaseNXButton(btn);
    }
    m_releaseButton1.reset();

    if(auto btn = m_releaseButton2.lock())
    {
        releaseNXButton(btn);
    }
    m_releaseButton2.reset();

    killTimer(m_nxButtonTimerId);
    m_nxButtonTimerId = 0;
  }
}

void BoardWidget::actionSelected(const Board::TileInfo* info)
{
  m_boardArea->setMouseMoveAction(BoardAreaWidget::MouseMoveAction::None);
  m_tileMoveStarted = false;
  m_tileResizeStarted = false;

  if(info)
  {
    m_tileRotates = info->rotates;
    validRotate(m_tileRotateLast, m_tileRotates);
    m_boardArea->setMouseMoveAction(BoardAreaWidget::MouseMoveAction::AddTile);
    m_boardArea->setMouseMoveTileRotate(m_tileRotateLast);
    m_boardArea->setMouseMoveTileId(info->tileId);
  }
  else
    m_tileRotates = 0;
}

void BoardWidget::keyPressEvent(QKeyEvent* event)
{
  switch(event->key())
  {
    case Qt::Key_Escape:
      if(m_tileMoveStarted || m_tileResizeStarted)
      {
        m_tileMoveStarted = false;
        m_tileResizeStarted = false;
        m_boardArea->setMouseMoveAction(BoardAreaWidget::MouseMoveAction::None);
      }
      else
        m_editActionNone->activate(QAction::Trigger);
      break;

    case Qt::Key_Left:
      rotateTile(true);
      break;

    case Qt::Key_Right:
      rotateTile();
      break;

    default:
      QWidget::keyPressEvent(event);
      break;
  }
}

void BoardWidget::timerEvent(QTimerEvent *e)
{
    if(e->timerId() == m_nxButtonTimerId)
    {
        stopTimerAndReleaseButtons();
        return;
    }

    QWidget::timerEvent(e);
}

void BoardWidget::rotateTile(bool ccw)
{
  if(m_tileRotates != 0)
  {
    m_boardArea->setMouseMoveTileRotate(
      ccw
      ? rotateCCW(m_boardArea->mouseMoveTileRotate(), m_tileRotates)
      : rotateCW(m_boardArea->mouseMoveTileRotate(), m_tileRotates));

    if(m_boardArea->mouseMoveTileHeight() != m_boardArea->mouseMoveTileWidth() && diff(m_tileRotateLast, m_boardArea->mouseMoveTileRotate()) == TileRotate::Deg90)
      m_boardArea->setMouseMoveTileSize(m_boardArea->mouseMoveTileHeight(), m_boardArea->mouseMoveTileWidth());

    m_tileRotateLast = m_boardArea->mouseMoveTileRotate();
  }
}

void BoardWidget::releaseNXButton(const std::shared_ptr<NXButtonRailTile>& nxButton)
{
  nxButton->setPressed(false);

  if(m_nxButtonPressed.lock() == nxButton)
  {
    m_nxButtonPressed.reset();
  }
}
