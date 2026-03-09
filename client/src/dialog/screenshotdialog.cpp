/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2025 Reinder Feenstra
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

#include "screenshotdialog.hpp"
#include <QToolBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QTimerEvent>
#include <QMenu>
#include <QProgressBar>
#include <QPushButton>
#include <QToolButton>
#include <QLineEdit>
#include <QTableView>
#include <QFileDialog>
#include <QSettings>
#include <QScreen>
#include "../mainwindow.hpp"
#include "../mdiarea.hpp"
#include "../board/boardwidget.hpp"
#include "../board/tilepainter.hpp"
#include "../network/abstractproperty.hpp"
#include "../network/error.hpp"
#include "../network/method.hpp"
#include "../network/object.hpp"
#include "../network/objectproperty.hpp"
#include "../settings/boardsettings.hpp"
#include "../settings/generalsettings.hpp"
#include "../subwindow/subwindow.hpp"
#include "../widget/object/abstracteditwidget.hpp"
#include "../widget/object/objecteditwidget.hpp"
#include "../widget/objectlist/objectlistwidget.hpp"
#include "../widget/objectlist/interfacelistwidget.hpp"
#include "../widget/objectlist/throttleobjectlistwidget.hpp"
#include "../widget/outputmapwidget.hpp"
#include "../widget/tile/tilewidget.hpp"
#include "../widget/throttle/throttlebutton.hpp"
#include "../widget/throttle/throttlewidget.hpp"
#include "../wizard/newboardwizard.hpp"
#include "../wizard/newworldwizard.hpp"
#include "../wizard/page/propertypage.hpp"

/*
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!                                                                           !!
!!  NOTE:                                                                    !!
!!    This is a bit hacky, it doesn't do much error checking,                !!
!!    it just asumes that everthing goes well :) #happyflow                  !!
!!                                                                           !!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/

ScreenShotDialog::ScreenShotDialog(MainWindow& mainWindow)
  : QDialog(&mainWindow)
  , m_mainWindow{mainWindow}
  , m_progressBar{new QProgressBar(this)}
  , m_label{new QLabel(this)}
  , m_start{new QPushButton("Start", this)}
  , m_languageCode{GeneralSettings::instance().language.value().left(2)}
{
  setWindowTitle("Screenshot creator");

  m_mainWindow.m_windowTitle = QStringLiteral("Traintastic"); // Get rid of version number

  m_label->setAlignment(Qt::AlignCenter);

  connect(m_start, &QPushButton::clicked, this, &ScreenShotDialog::start);

  auto* mainLayout = new QVBoxLayout();
  mainLayout->addWidget(m_progressBar);
  mainLayout->addWidget(m_label);
  auto* h = new QHBoxLayout();
  h->addStretch();
  h->addWidget(m_start);
  h->addStretch();
  mainLayout->addLayout(h);
  setLayout(mainLayout);
}

void ScreenShotDialog::timerEvent(QTimerEvent* event)
{
  if(event->timerId() == m_stepTimer)
  {
    if(m_steps.empty())
    {
      killTimer(m_stepTimer);
      m_stepTimer = 0;
      m_label->setText(QStringLiteral("Done!"));
    }
    else if(m_steps.front()())
    {
      m_steps.pop();
      m_progressBar->setValue(m_progressBar->value() + 1);
      m_label->setText(QString("%1 / %2").arg(m_progressBar->value()).arg(m_progressBar->maximum()));
    }
  }
}

void ScreenShotDialog::start()
{
  QSettings settings;
  m_outputDir.setPath(QFileDialog::getExistingDirectory(this, "Select output directory", settings.value("ScreenshotCreator/OutputDirectory", QDir::homePath()).toString()));
  if(!m_outputDir.exists())
  {
    return;
  }

  settings.setValue("ScreenshotCreator/OutputDirectory", m_outputDir.path());

  // Startup:
  m_steps.push(
    [this]()
    {
      return m_mainWindow.connection().operator bool();
    });
  m_steps.push(
    [this]()
    {
      if(m_mainWindow.world())
      {
        m_mainWindow.m_actionCloseWorld->trigger();
        return false;
      }
      return true;
    });

  // No world:
  m_steps.push(
    [this]()
    {
      saveMainWindowImage(QStringLiteral("getting-started/traintastic-startup-no-world.png"));
      return true;
    });

  // Create new world:
  m_steps.push(
    [this]()
    {
       m_mainWindow.m_actionNewWorld->trigger();
      return true;
    });
  m_steps.push(
    [this]()
    {
      return m_mainWindow.world().operator bool();
    });

  // New world wizard:
  m_steps.push(
    [this]()
    {
      m_mainWindow.m_wizard.newWorld->next();
      return true;
    });
  m_steps.push(
    [this]()
    {
      m_mainWindow.m_wizard.newWorld->button(QWizard::NextButton)->setFocus();
      m_mainWindow.m_world->setPropertyValue("name", QStringLiteral("My First World"));
      return true;
    });
  m_steps.push(
    [this]()
    {
      saveDialogImage(m_mainWindow.m_wizard.newWorld.get(), QStringLiteral("getting-started/new-world-wizard-set-name.png"));
      m_mainWindow.m_wizard.newWorld->next();
      return true;
    });
  m_steps.push(
    [this]()
    {
      saveDialogImage(m_mainWindow.m_wizard.newWorld.get(), QStringLiteral("getting-started/new-world-wizard-select-scale.png"));
      m_mainWindow.m_wizard.newWorld->next();
      return true;
    });
  m_steps.push(
    [this]()
    {
      saveDialogImage(m_mainWindow.m_wizard.newWorld.get(), QStringLiteral("getting-started/new-world-wizard-finish.png"));
      m_mainWindow.m_wizard.newWorld->accept();
      return true;
    });

  // Interface list:
  m_steps.push(
    [this]()
    {
      m_mainWindow.m_menuObjects->actions()[0]->menu()->actions()[0]->trigger(); // Object -> Hardware -> Interfaces
      return true;
    });
  m_steps.push(
    [this]()
    {
      if(auto* w = getSubWindow(QStringLiteral("world.interfaces")))
      {
        w->move(10, 10);
        w->resize(400, 300);
        saveWidgetImage(w, QStringLiteral("interface/interface-list-empty.png"));
        auto* list = static_cast<InterfaceListWidget*>(w->widget());
        list->m_createMenu->actions()[3]->trigger();
        return true;
      }
      return false;
    });
  m_steps.push(
    [this]()
    {
      if(auto* w = getSubWindow(QStringLiteral("world.interfaces")))
      {
        w->close();
        return true;
      }
      return false;
    });

  // Trains and Rail vehicles:
  m_steps.push(
    [this]()
    {
      m_mainWindow.m_menuObjects->actions()[4]->trigger(); // Object -> Trains
      return true;
    });
  m_steps.push(
    [this]()
    {
      if(auto* w = m_mainWindow.m_trainAndRailVehiclesSubWindow)
      {
        w->move(10, 10);
        w->resize(500, 300);
        saveWidgetImage(w, QStringLiteral("getting-started/train-list-empty.png"));
        static_cast<QTabWidget*>(w->widget())->setCurrentIndex(1);
        saveWidgetImage(w, QStringLiteral("getting-started/rail-vehicle-list-empty.png"));
        return true;
      }
      return false;
    });
  m_steps.push(
    [this]()
    {
      if(auto* w = m_mainWindow.m_trainAndRailVehiclesSubWindow)
      {
        auto* list = static_cast<ObjectListWidget*>(static_cast<QTabWidget*>(w->widget())->currentWidget()->layout()->itemAt(0)->widget());
        list->m_buttonCreate->menu()->actions()[0]->trigger(); // create locomotive
        return true;
      }
      return false;
    });
  m_steps.push(
    [this]()
    {
      if(auto* w = getSubWindow(QStringLiteral("vehicle_1")))
      {
        w->move(10, 10);
        w->resize(400, 300);
        saveWidgetImage(w, QStringLiteral("getting-started/locomotive-general.png"));
        static_cast<QTabWidget*>(w->widget()->layout()->itemAt(0)->widget())->setCurrentIndex(1);
        saveWidgetImage(w, QStringLiteral("getting-started/locomotive-decoder.png"));
        static_cast<QTabWidget*>(w->widget()->layout()->itemAt(0)->widget())->setCurrentIndex(2);
        saveWidgetImage(w, QStringLiteral("getting-started/locomotive-functions.png"));
        w->close();
        return true;
      }
      return false;
    });
  m_steps.push(
    [this]()
    {
      if(auto* w = m_mainWindow.m_trainAndRailVehiclesSubWindow)
      {
        static_cast<QTabWidget*>(w->widget())->setCurrentIndex(0);
        auto* list = static_cast<ObjectListWidget*>(static_cast<QTabWidget*>(w->widget())->currentWidget()->layout()->itemAt(0)->widget());
        list->m_actionCreate->trigger(); // create train
        return true;
      }
      return false;
    });
  m_steps.push(
    [this]()
    {
      if(auto* w = getSubWindow(QStringLiteral("train_1")))
      {
        w->move(400, 10);
        w->resize(400, 300);
        saveWidgetImage(w, QStringLiteral("getting-started/train-general.png"));
        static_cast<QTabWidget*>(w->widget()->layout()->itemAt(0)->widget())->setCurrentIndex(1);
        saveWidgetImage(w, QStringLiteral("getting-started/train-vehicles.png"));
        auto* list = static_cast<ObjectListWidget*>(static_cast<QTabWidget*>(w->widget()->layout()->itemAt(0)->widget())->currentWidget()->layout()->itemAt(0)->widget());
        list->object()->getMethod("add")->call("vehicle_1");
        return true;
      }
      return false;
    });
  m_steps.push(
    [this]()
    {
      if(auto* w = getSubWindow(QStringLiteral("train_1")))
      {
        w->close();
        return true;
      }
      return false;
    });
  m_steps.push(
    [this]()
    {
      m_mainWindow.m_worldEditAction->setChecked(false); // switch to operate mode
      return true;
    });
  m_steps.push(
    [this]()
    {
      if(auto* w = m_mainWindow.m_trainAndRailVehiclesSubWindow)
      {
        auto* list = static_cast<ThrottleObjectListWidget*>(static_cast<QTabWidget*>(w->widget())->currentWidget()->layout()->itemAt(0)->widget());
        auto* table = static_cast<QTableView*>(list->layout()->itemAt(1)->widget());
        table->selectRow(0);
        list->m_actionThrottle->trigger();
        w->close();
        return true;
      }
      return false;
    });
  m_steps.push(
    [this]()
    {
      if(auto* w = getSubWindow(QStringLiteral("train_1"), SubWindowType::Throttle))
      {
        m_mainWindow.m_mdiArea->setActiveSubWindow(w);
        w->move(10, 10);
        w->resize(350, 350);
        return true;
      }
      return false;
    });
  m_steps.push(
    [this]()
    {
      if(auto* w = getSubWindow(QStringLiteral("train_1"), SubWindowType::Throttle))
      {
        saveWidgetImage(w, QStringLiteral("getting-started/train-throttle.png"));
        emit static_cast<ThrottleWidget*>(w->widget())->m_throttleAction->clicked(); // acquire
        return true;
      }
      return false;
    });
  m_steps.push(
    [this]()
    {
      if(auto* w = getSubWindow(QStringLiteral("train_1"), SubWindowType::Throttle))
      {
        saveWidgetImage(w, QStringLiteral("getting-started/train-throttle-acquired.png"));
        w->close();
        return true;
      }
      return false;
    });
  m_steps.push(
    [this]()
    {
      m_mainWindow.m_worldEditAction->setChecked(true); // switch to edit mode
      return true;
    });

  // Board list:
  m_steps.push(
    [this]()
    {
      m_mainWindow.m_menuObjects->actions()[1]->trigger(); // Object -> Trains
      return true;
    });
  m_steps.push(
    [this]()
    {
      if(auto* w = getSubWindow(QStringLiteral("world.boards")))
      {
        w->move(10, 10);
        w->resize(400, 300);
        return true;
      }
      return false;
    });
  m_steps.push(
    [this]()
    {
      if(auto* w = getSubWindow(QStringLiteral("world.boards")))
      {
        saveWidgetImage(w, QStringLiteral("getting-started/board-list-empty.png"));
        static_cast<ObjectListWidget*>(w->widget())->m_actionCreate->trigger();
        return true;
      }
      return false;
    });
  m_steps.push(
    [this]()
    {
      if(auto* w = getSubWindow(QStringLiteral("world.boards")))
      {
        w->close();
        return true;
      }
      return false;
    });
  m_steps.push(
    [this]()
    {
      if(!m_mainWindow.m_wizard.newBoardWizards.empty())
      {
        m_mainWindow.m_wizard.newBoardWizards.front()->next();
        return true;
      }
      return false;
    });
  m_steps.push(
    [this]()
    {
      if(!m_mainWindow.m_wizard.newBoardWizards.empty())
      {
        m_mainWindow.m_wizard.newBoardWizards.front()->next();
        return true;
      }
      return false;
    });
  m_steps.push(
    [this]()
    {
      if(!m_mainWindow.m_wizard.newBoardWizards.empty())
      {
        auto* wizard = m_mainWindow.m_wizard.newBoardWizards.front();
        wizard->button(QWizard::NextButton)->setFocus();
        wizard->m_name->setValueString(QStringLiteral("Example layout"));
        return true;
      }
      return false;
    });
  m_steps.push(
    [this]()
    {
      if(!m_mainWindow.m_wizard.newBoardWizards.empty())
      {
        m_mainWindow.m_wizard.newBoardWizards.front()->next();
        return true;
      }
      return false;
    });
  m_steps.push(
    [this]()
    {
      if(!m_mainWindow.m_wizard.newBoardWizards.empty())
      {
        m_mainWindow.m_wizard.newBoardWizards.front()->accept();
        return true;
      }
      return false;
    });
  m_steps.push(
    [this]()
    {
      if(auto* w = getSubWindow(QStringLiteral("board_1"), SubWindowType::Board))
      {
        w->move(10, 10);
        w->resize(600, 500);
        return true;
      }
      return false;
    });
  m_steps.push(
    [this]()
    {
      if(auto* w = getSubWindow(QStringLiteral("board_1"), SubWindowType::Board))
      {
        auto* boardWidget = static_cast<BoardWidget*>(w->layout()->itemAt(0)->widget());
        auto& board = boardWidget->board();

        static auto ignoreCallback = [](const bool&, std::optional<const Error>){};
        static const QString straight{QStringLiteral("board_tile.rail.straight")};
        static const QString curve45{QStringLiteral("board_tile.rail.curve_45")};
        static const QString turnoutLeft45{QStringLiteral("board_tile.rail.turnout_left_45")};
        static const QString turnoutRight45{QStringLiteral("board_tile.rail.turnout_right_45")};
        static const QString block{QStringLiteral("board_tile.rail.block")};

        (void)board.addTile(0, 2, TileRotate::Deg0, straight, false, ignoreCallback);
        (void)board.addTile(0, 1, TileRotate::Deg225, curve45, false, ignoreCallback);
        (void)board.addTile(1, 0, TileRotate::Deg270, curve45, false, ignoreCallback);
        (void)board.addTile(2, 0, TileRotate::Deg90, turnoutRight45, false, ignoreCallback);
        (void)board.addTile(3, 0, TileRotate::Deg90, straight, false, ignoreCallback);
        (void)board.addTile(4, 0, TileRotate::Deg90, straight, false, ignoreCallback);
        (void)board.addTile(5, 0, TileRotate::Deg90, block, false, ignoreCallback);
        (void)board.resizeTile(5, 0, 5, 1, ignoreCallback);
        (void)board.addTile(10, 0, TileRotate::Deg90, straight, false, ignoreCallback);
        (void)board.addTile(11, 0, TileRotate::Deg90, straight, false, ignoreCallback);
        (void)board.addTile(12, 0, TileRotate::Deg270, turnoutLeft45, false, ignoreCallback);
        (void)board.addTile(13, 0, TileRotate::Deg315, curve45, false, ignoreCallback);
        (void)board.addTile(14, 1, TileRotate::Deg0, curve45, false, ignoreCallback);
        (void)board.addTile(14, 2, TileRotate::Deg0, straight, false, ignoreCallback);
        (void)board.addTile(14, 3, TileRotate::Deg0, block, false, ignoreCallback);
        (void)board.resizeTile(14, 3, 1, 5, ignoreCallback);
        (void)board.addTile(14, 8, TileRotate::Deg0, straight, false, ignoreCallback);
        (void)board.addTile(14, 9, TileRotate::Deg45, curve45, false, ignoreCallback);
        (void)board.addTile(13, 10, TileRotate::Deg90, curve45, false, ignoreCallback);
        (void)board.addTile(12, 10, TileRotate::Deg270, turnoutRight45, false, ignoreCallback);
        (void)board.addTile(11, 10, TileRotate::Deg90, straight, false, ignoreCallback);
        (void)board.addTile(10, 10, TileRotate::Deg90, straight, false, ignoreCallback);
        (void)board.addTile(5, 10, TileRotate::Deg90, block, false, ignoreCallback);
        (void)board.resizeTile(5, 10, 5, 1, ignoreCallback);
        (void)board.addTile(4, 10, TileRotate::Deg90, straight, false, ignoreCallback);
        (void)board.addTile(3, 10, TileRotate::Deg90, straight, false, ignoreCallback);
        (void)board.addTile(2, 10, TileRotate::Deg90, straight, false, ignoreCallback);
        (void)board.addTile(1, 10, TileRotate::Deg135, curve45, false, ignoreCallback);
        (void)board.addTile(0, 9, TileRotate::Deg180, curve45, false, ignoreCallback);
        (void)board.addTile(0, 8, TileRotate::Deg0, straight, false, ignoreCallback);
        (void)board.addTile(0, 3, TileRotate::Deg0, block, false, ignoreCallback);
        (void)board.resizeTile(0, 3, 1, 5, ignoreCallback);

        (void)board.addTile(3, 1, TileRotate::Deg135, curve45, false, ignoreCallback);
        (void)board.addTile(4, 1, TileRotate::Deg90, straight, false, ignoreCallback);
        (void)board.addTile(5, 1, TileRotate::Deg90, block, false, ignoreCallback);
        (void)board.resizeTile(5, 1, 5, 1, ignoreCallback);
        (void)board.addTile(10, 1, TileRotate::Deg90, straight, false, ignoreCallback);
        (void)board.addTile(11, 1, TileRotate::Deg90, curve45, false, ignoreCallback);

        (void)board.addTile(4, 9, TileRotate::Deg270, QStringLiteral("board_tile.rail.buffer_stop"), false, ignoreCallback);
        (void)board.addTile(5, 9, TileRotate::Deg90, block, false, ignoreCallback);
        (void)board.resizeTile(5, 9, 5, 1, ignoreCallback);
        (void)board.addTile(10, 9, TileRotate::Deg90, straight, false, ignoreCallback);
        (void)board.addTile(11, 9, TileRotate::Deg315, curve45, false, ignoreCallback);

        return true;
      }
      return false;
    });
  m_steps.push(
    [this]()
    {
      if(auto* w = getSubWindow(QStringLiteral("board_1"), SubWindowType::Board))
      {
        saveWidgetImage(w, QStringLiteral("getting-started/board-example.png"));
        w->close();
        return true;
      }
      return false;
    });

  // Turnout dialog:
  m_steps.push(
    [this]()
    {
      m_mainWindow.showObject("turnout_1");
      return true;
    });
  m_steps.push(
    [this]()
    {
      if(auto* w = getSubWindow(QStringLiteral("turnout_1")))
      {
        w->resize(400, 350);
        auto* map = static_cast<OutputMapWidget*>(static_cast<TileWidget*>(w->widget())->m_tabs->widget(0));
        map->m_object->getObjectProperty("interface")->setByObjectId("loconet_1");
        return true;
      }
      return false;
    });
  m_steps.push(
    [this]()
    {
      if(auto* w = getSubWindow(QStringLiteral("turnout_1")))
      {
        auto* map = static_cast<OutputMapWidget*>(static_cast<TileWidget*>(w->widget())->m_tabs->widget(0));
        if(map->m_object->getObjectProperty("interface")->hasObject())
        {
          saveWidgetImage(w, QStringLiteral("getting-started/turnout-general.png"));
          w->close();
          return true;
        }
      }
      return false;
    });

  // Board tiles:
  m_steps.push(
    [this]()
    {
      static const std::array<std::tuple<const BoardColorScheme&, const char*>, 2> schemes{
        std::make_tuple<const BoardColorScheme&, const char*>(BoardColorScheme::light, "light"),
        std::make_tuple<const BoardColorScheme&, const char*>(BoardColorScheme::dark, "dark")
      };

      static constexpr std::array<std::tuple<TileId, const char*>, 32> tiles{
        std::make_tuple(TileId::RailStraight, "rail/straight.png"),
        std::make_tuple(TileId::RailCurve45, "rail/curve45.png"),
        std::make_tuple(TileId::RailCurve90, "rail/curve90.png"),
        std::make_tuple(TileId::RailCross45, "rail/cross45.png"),
        std::make_tuple(TileId::RailCross90, "rail/cross90.png"),
        std::make_tuple(TileId::RailTurnoutLeft45, "rail/turnoutleft45.png"),
        std::make_tuple(TileId::RailTurnoutRight45, "rail/turnoutright45.png"),
        std::make_tuple(TileId::RailTurnoutWye, "rail/turnoutwye.png"),
        std::make_tuple(TileId::RailTurnout3Way, "rail/turnout3way.png"),
        std::make_tuple(TileId::RailTurnoutSingleSlip, "rail/turnoutsingleslip.png"),
        std::make_tuple(TileId::RailTurnoutDoubleSlip, "rail/turnoutdoubleslip.png"),
        std::make_tuple(TileId::RailSignal2Aspect, "rail/signal2aspect.png"),
        std::make_tuple(TileId::RailSignal3Aspect, "rail/signal3aspect.png"),
        std::make_tuple(TileId::RailBufferStop, "rail/bufferstop.png"),
        std::make_tuple(TileId::RailSensor, "rail/sensor.png"),
        std::make_tuple(TileId::RailBlock, "rail/block.png"),
        std::make_tuple(TileId::RailTurnoutLeft90, "rail/turnoutleft90.png"),
        std::make_tuple(TileId::RailTurnoutRight90, "rail/turnoutright90.png"),
        std::make_tuple(TileId::RailTurnoutLeftCurved, "rail/turnoutleftcurved.png"),
        std::make_tuple(TileId::RailTurnoutRightCurved, "rail/turnoutrightcurved.png"),
        std::make_tuple(TileId::RailBridge45Left, "rail/bridge45left.png"),
        std::make_tuple(TileId::RailBridge45Right, "rail/bridge45right.png"),
        std::make_tuple(TileId::RailBridge90, "rail/bridge90.png"),
        std::make_tuple(TileId::RailTunnel, "rail/tunnel.png"),
        std::make_tuple(TileId::RailOneWay, "rail/oneway.png"),
        std::make_tuple(TileId::RailDirectionControl, "rail/directioncontrol.png"),
        std::make_tuple(TileId::PushButton, "misc/pushbutton.png"),
        std::make_tuple(TileId::RailLink, "rail/link.png"),
        std::make_tuple(TileId::RailDecoupler, "rail/decoupler.png"),
        std::make_tuple(TileId::RailNXButton, "rail/nxbutton.png"),
        std::make_tuple(TileId::Label, "misc/label.png"),
        std::make_tuple(TileId::Switch, "misc/switch.png"),
      };

      const bool turnoutDrawStateOrg = BoardSettings::instance().turnoutDrawState;
      BoardSettings::instance().turnoutDrawState = true;

      QPixmap pixmap(QSize{24, 24});
      const QRectF rect{{0, 0}, pixmap.size()};
      for(auto& [scheme, schemeName] : schemes)
      {
        for(auto& [tileId, filename] : tiles)
        {
          pixmap.fill(Qt::transparent);
          QPainter painter(&pixmap);
          painter.setRenderHint(QPainter::Antialiasing, true);
          TilePainter tilePainter{painter, pixmap.size().width(), scheme};
          switch(tileId)
          {
            case TileId::RailBlock:
              tilePainter.draw(tileId, rect, TileRotate::Deg90);
              break;

            case TileId::RailSignal2Aspect:
            case TileId::RailSignal3Aspect:
              tilePainter.drawSignal(tileId, rect, TileRotate::Deg0, false, SignalAspect::Stop);
              break;

            default:
              tilePainter.draw(tileId, rect, TileRotate::Deg0);
              break;
          }
          const QString outputPath = m_outputDir.filePath(QString("overrides/assets/images/board/tiles/%1/%2").arg(schemeName).arg(filename));
          QDir().mkpath(QFileInfo(outputPath).absolutePath());
          pixmap.save(outputPath);
        }
      }

      BoardSettings::instance().turnoutDrawState = turnoutDrawStateOrg;

      return true;
    });

  // Lua scripts list:
  m_steps.push(
    [this]()
    {
      m_mainWindow.m_menuObjects->actions()[5]->trigger(); // Object -> Lua scripts
      return true;
    });
  m_steps.push(
    [this]()
    {
      if(auto* w = getSubWindow(QStringLiteral("world.lua_scripts")))
      {
        w->move(10, 10);
        w->resize(400, 300);
        saveWidgetImage(w, QStringLiteral("lua/lua-script-list-empty.png"));
        static_cast<ObjectListWidget*>(w->widget())->m_actionCreate->trigger();
        return true;
      }
      return false;
    });
  m_steps.push(
    [this]()
    {
      if(auto* w = getSubWindow(QStringLiteral("script_1")))
      {
        w->move(420, 10);
        w->resize(400, 300);
        saveWidgetImage(w, QStringLiteral("lua/lua-script-editor.png"));
        m_mainWindow.m_mdiArea->closeAllSubWindows();
        return true;
      }
      return false;
    });

  // Icons:
  m_steps.push(
    [this]()
    {
      static constexpr std::array<const char*, 2> schemes{"light", "dark"};
      static constexpr std::array<const char*, 23> icons{
        "circle/add",
        "add",
        "board",
        "delete",
        "down",
        "edit",
        "highlight_zone",
        "interface_state.error",
        "interface_state.initializing",
        "interface_state.offline",
        "interface_state.online",
        "mouse",
        "move_tile",
        "offline",
        "online",
        "power_off",
        "power_on",
        "remove",
        "resize_tile",
        "run",
        "swap",
        "train",
        "up",
      };

      const QSize sz{24, 24};
      for(const char* scheme : schemes)
      {
        for(const char* icon : icons)
        {
          const QString outputPath = m_outputDir.filePath(QString("overrides/assets/images/icons/%1/%2.png").arg(scheme).arg(icon));
          QDir().mkpath(QFileInfo(outputPath).absolutePath());
          QIcon(QString(":/%1/%2.svg").arg(scheme).arg(icon)).pixmap(sz).save(outputPath);
        }
      }

      return true;
    });

  m_start->setEnabled(false);
  m_progressBar->setRange(0, static_cast<int>(m_steps.size()));
  m_progressBar->setValue(0);
  m_label->setText(QString("%1 / %2").arg(m_progressBar->value()).arg(m_progressBar->maximum()));

  using namespace std::chrono_literals;
  m_stepTimer = startTimer(500ms);
}

SubWindow* ScreenShotDialog::getSubWindow(const QString& id, SubWindowType type)
{
  if(auto it = m_mainWindow.m_subWindows.find(SubWindow::windowId(type, id)); it != m_mainWindow.m_subWindows.end())
  {
    return it.value();
  }
  return nullptr;
}

void ScreenShotDialog::savePixmap(QPixmap pixmap, const QString& filename)
{
  const QString outputPath = m_outputDir.filePath(QString("docs/%1/assets/images/%2").arg(m_languageCode).arg(filename));
  QDir().mkpath(QFileInfo(outputPath).absolutePath());
  pixmap.save(outputPath);
}

void ScreenShotDialog::saveWidgetImage(QWidget* widget, const QString& filename)
{
  savePixmap(widget->grab(), filename);
}

void ScreenShotDialog::saveDialogImage(QDialog* dialog, const QString& filename)
{
  savePixmap(dialog->screen()->grabWindow(0).copy(dialog->frameGeometry()), filename);
}

void ScreenShotDialog::saveMainWindowImage(const QString& filename)
{
  savePixmap(m_mainWindow.screen()->grabWindow(0).copy(m_mainWindow.frameGeometry()), filename);
}
