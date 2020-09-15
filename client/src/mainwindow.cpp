/**
 * client/src/mainwindow.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

#include "mainwindow.hpp"
#include <QMenuBar>
#include <QStatusBar>
#include <QMdiSubWindow>
#include <QVBoxLayout>
#include <QToolBar>
#include <QApplication>
#include <QMessageBox>
#include <QSettings>
#include <QApplication>
#include <QSplitter>
#include "mdiarea.hpp"
#include "dialog/connectdialog.hpp"
#include "dialog/settingsdialog.hpp"
#include "dialog/worldlistdialog.hpp"
#include "network/connection.hpp"
#include "network/object.hpp"
#include "network/property.hpp"
#include "network/method.hpp"
//#include "subwindow/objecteditsubwindow.hpp"
#include "subwindow/objectsubwindow.hpp"
#include "widget/serverconsolewidget.hpp"


#include <QDesktopServices>
#include <traintastic/locale/locale.hpp>
#include <traintastic/codename.hpp>

#define SETTING_PREFIX "mainwindow/"
#define SETTING_GEOMETRY SETTING_PREFIX "geometry"
#define SETTING_WINDOWSTATE SETTING_PREFIX "windowstate"

static void setMenuEnabled(QMenu* menu, bool enabled)
{
  menu->setEnabled(enabled);
  for(QAction* action : menu->actions())
  {
    if(action->menu())
      setMenuEnabled(action->menu(), enabled);
    action->setEnabled(enabled);
  }
}


MainWindow::MainWindow(QWidget* parent) :
  QMainWindow(parent),
  m_splitter{new QSplitter(Qt::Vertical, this)},
  m_mdiArea{new MdiArea(m_splitter)},
  m_serverConsole{nullptr}
{
  instance = this;

  updateWindowTitle();

  QAction* actFullScreen;

  m_mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  m_mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  // build the main menu
  {
    setMenuBar(new QMenuBar());
    QMenu* menu;
    //QMenu* subMenu;
    //QAction* act;

    menu = menuBar()->addMenu(Locale::tr("qtapp.mainmenu:file"));
    m_actionConnectToServer = menu->addAction(Locale::tr("qtapp.mainmenu:connect_to_server") + "...", this, &MainWindow::connectToServer);
    m_actionDisconnectFromServer = menu->addAction(Locale::tr("qtapp.mainmenu:disconnect_from_server"), this, &MainWindow::disconnectFromServer);
    menu->addSeparator();
    m_actionNewWorld = menu->addAction(QIcon(":/dark/world_new.svg"), Locale::tr("qtapp.mainmenu:new_world"),
      [this]()
      {
        if(m_connection)
          if(const ObjectPtr& traintastic = m_connection->traintastic())
            if(Method* method = traintastic->getMethod("new_world"))
              method->call();
      });
    m_actionNewWorld->setShortcut(QKeySequence::New);
    m_actionLoadWorld = menu->addAction(QIcon(":/dark/world_load.svg"), Locale::tr("qtapp.mainmenu:load_world") + "...", this, &MainWindow::loadWorld);
    m_actionLoadWorld->setShortcut(QKeySequence::Open);
    m_actionSaveWorld = menu->addAction(QIcon(":/dark/world_save.svg"), Locale::tr("qtapp.mainmenu:save_world"),
      [this]()
      {
        if(m_world)
          if(Method* method = m_world->getMethod("save"))
            method->call();
      });
    m_actionSaveWorld->setShortcut(QKeySequence::Save);
    menu->addSeparator();
    m_actionImportWorld = menu->addAction(QIcon(":/dark/world_import.svg"), Locale::tr("qtapp.mainmenu:import_world") + "...", this, &MainWindow::importWorld);
    m_actionExportWorld = menu->addAction(QIcon(":/dark/world_export.svg"), Locale::tr("qtapp.mainmenu:export_world") + "...", this, &MainWindow::exportWorld);
    menu->addSeparator();
    menu->addAction(Locale::tr("qtapp.mainmenu:quit"), this, &MainWindow::close)->setShortcut(QKeySequence::Quit);

    menu = menuBar()->addMenu(Locale::tr("qtapp.mainmenu:view"));
    actFullScreen = menu->addAction(Locale::tr("qtapp.mainmenu:fullscreen"), this, &MainWindow::toggleFullScreen);
    actFullScreen->setCheckable(true);
    actFullScreen->setShortcut(Qt::Key_F11);
    m_actionServerConsole = menu->addAction(Locale::tr("qtapp.mainmenu:server_console") + "...", this, &MainWindow::toggleConsole);//[this](){ showObject("traintastic.console"); });
    m_actionServerConsole->setCheckable(true);
    m_actionServerConsole->setShortcut(Qt::Key_F12);

    m_menuObjects = menuBar()->addMenu(Locale::tr("qtapp.mainmenu:objects"));
    menu = m_menuObjects->addMenu(QIcon(":/dark/hardware.svg"), Locale::tr("qtapp.mainmenu:hardware"));
    menu->addAction(Locale::tr("world:command_stations") + "...", [this](){ showObject("world.command_stations", Locale::tr("world:command_stations")); });
    menu->addAction(Locale::tr("world:decoders") + "...", [this](){ showObject("world.decoders", Locale::tr("world:decoders")); });
    menu->addAction(Locale::tr("world:inputs") + "...", [this](){ showObject("world.inputs", Locale::tr("world:inputs")); });
    menu->addAction(Locale::tr("world:controllers") + "...", [this](){ showObject("world.controllers", Locale::tr("world:controllers")); });
    m_menuObjects->addAction(Locale::tr("world:clock") + "...", [this](){ showObject("world.clock", Locale::tr("world:clock")); });
    m_menuObjects->addAction(Locale::tr("world:trains") + "...", [this](){ showObject("world.trains", Locale::tr("world:trains")); });
    m_menuObjects->addAction(Locale::tr("world:rail_vehicles") + "...", [this](){ showObject("world.rail_vehicles", Locale::tr("world:rail_vehicles")); });
    m_actionLuaScript = m_menuObjects->addAction(QIcon(":/dark/lua.svg"), Locale::tr("world:lua_scripts") + "...", [this](){ showObject("world.lua_scripts", Locale::tr("world:lua_scripts")); });

    menu = menuBar()->addMenu(Locale::tr("qtapp.mainmenu:tools"));
    menu->addAction(Locale::tr("qtapp.mainmenu:settings") + "...",
      [this]()
      {
        std::unique_ptr<SettingsDialog> d = std::make_unique<SettingsDialog>(this);
        d->exec();
      });
    menu->addSeparator();
    m_menuServer = menu->addMenu(Locale::tr("qtapp.mainmenu:server"));
    m_actionServerSettings = m_menuServer->addAction(Locale::tr("qtapp.mainmenu:server_settings") + "...", this,
      [this]()
      {
        showObject("traintastic.settings", Locale::tr("qtapp.mainmenu:server_settings"));
      });
    m_menuServer->addSeparator();
    m_actionServerRestart = m_menuServer->addAction(Locale::tr("qtapp.mainmenu:restart_server"), this,
      [this]()
      {
        if(QMessageBox::question(this, Locale::tr("qtapp.mainmenu:restart_server"), Locale::tr("qtapp.mainmenu:restart_server_question"), Locale::tr("qtapp.message_box:yes"), Locale::tr("qtapp.message_box:no"), "", 0, 1) != 0)
          return;

        if(m_connection)
          if(const ObjectPtr& traintastic = m_connection->traintastic())
            if(Method* method = traintastic->getMethod("restart"))
              method->call();
      });
    m_actionServerShutdown = m_menuServer->addAction(Locale::tr("qtapp.mainmenu:shutdown_server"), this,
      [this]()
      {
        if(QMessageBox::question(this, Locale::tr("qtapp.mainmenu:shutdown_server"), Locale::tr("qtapp.mainmenu:shutdown_server_question"), Locale::tr("qtapp.message_box:yes"), Locale::tr("qtapp.message_box:no"), "", 0, 1) != 0)
          return;

        if(m_connection)
          if(const ObjectPtr& traintastic = m_connection->traintastic())
            if(Method* method = traintastic->getMethod("shutdown"))
              method->call();
      });

    menu = menuBar()->addMenu(Locale::tr("qtapp.mainmenu:help"));
    menu->addAction(Locale::tr("qtapp.mainmenu:help"), [](){ QDesktopServices::openUrl("https://traintastic.org/manual?version=" + QApplication::applicationVersion()); })->setShortcut(QKeySequence::HelpContents);
    //menu->addSeparator();
    //menu->addAction(Locale::tr("qtapp.mainmenu:about_qt") + "...", qApp, &QApplication::aboutQt);
    menu->addAction(Locale::tr("qtapp.mainmenu:about") + "...", this, &MainWindow::showAbout);
  }

  QToolBar* toolbar = new QToolBar();
  m_actionEmergencyStop = toolbar->addAction(QIcon(":/dark/emergency_stop.svg"), Locale::tr("world:emergency_stop"),
    [this]()
    {
     if(m_world)
       if(Method* method = m_world->getMethod("emergency_stop"))
         method->call();
    });
  toolbar->addSeparator();
  m_actionTrackPowerOff = toolbar->addAction(QIcon(":/dark/track_power_off.svg"), Locale::tr("world:track_power_off"),
    [this]()
    {
     if(m_world)
       if(Method* method = m_world->getMethod("track_power_off"))
         method->call();
    });
  m_actionTrackPowerOn = toolbar->addAction(QIcon(":/dark/track_power_on.svg"), Locale::tr("world:track_power_on"),
    [this]()
    {
     if(m_world)
       if(Method* method = m_world->getMethod("track_power_on"))
         method->call();
    });
  toolbar->addSeparator();


  //m_actionGroupMode = new QActionGroup(this);
  //m_actionGroupMode->setExclusive(true);
  //m_actionModeRun = toolbar->addAction(QIcon(":/dark/run.svg"), tr("Run"), [this](){ setMode(TraintasticMode::Run); });
  //m_actionModeRun->setCheckable(true);
  //m_actionGroupMode->addAction(m_actionModeRun);
  //m_actionModeStop = toolbar->addAction(QIcon(":/dark/stop.svg"), tr("Stop"), [this](){ setMode(TraintasticMode::Stop); });
  //m_actionModeStop->setCheckable(true);
  //m_actionGroupMode->addAction(m_actionModeStop);

  QWidget* spacer = new QWidget(this);
  spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  spacer->show();
  toolbar->addWidget(spacer);

  m_actionEdit = toolbar->addAction(QIcon(":/dark/edit.svg"), Locale::tr("world:edit"),
    [this](bool checked)
    {
      if(m_world)
        if(AbstractProperty* property = m_world->getProperty("edit"))
          property->setValueBool(checked);
    });
  m_actionEdit->setCheckable(true);
  //m_actionGroupMode->addAction(m_actionModeEdit);
  //toolbar->addSeparator();
  //toolbar->addAction(m_actionHardware);
  //toolbar->addAction(m_actionLua);

  QVBoxLayout* l = new QVBoxLayout();
  l->setMargin(0);
  l->addWidget(toolbar);
  l->addWidget(m_mdiArea);

  QWidget* w = new QWidget();
  w->setLayout(l);
  m_splitter->addWidget(w);
  m_splitter->setCollapsible(0, false);

  setCentralWidget(m_splitter);
  setStatusBar(new QStatusBar());

  QSettings settings;
  if(settings.contains(SETTING_GEOMETRY))
    restoreGeometry(settings.value(SETTING_GEOMETRY).toByteArray());
  if(settings.contains(SETTING_WINDOWSTATE))
    setWindowState(static_cast<Qt::WindowState>(settings.value(SETTING_WINDOWSTATE).toInt()));
  actFullScreen->setChecked(isFullScreen());

  connectionStateChanged();
}

MainWindow::~MainWindow()
{
  for(QMdiSubWindow* window : m_mdiSubWindows)
    disconnect(window, &QMdiSubWindow::destroyed, nullptr, nullptr);
}

void MainWindow::connectToServer()
{
  if(m_connection)
    return;

  std::unique_ptr<ConnectDialog> d = std::make_unique<ConnectDialog>(this);
  if(d->exec() == QDialog::Accepted)
  {
    m_connection = d->connection();
    connect(m_connection.data(), &Connection::worldChanged,
      [this]()
      {
        worldChanged();
        updateActions();
      });
    connect(m_connection.data(), &Connection::stateChanged, this, &MainWindow::connectionStateChanged);
    worldChanged();
    connectionStateChanged();
  }
}

void MainWindow::disconnectFromServer()
{
  if(m_connection)
    m_connection->disconnectFromHost();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
  QSettings settings;
  settings.setValue(SETTING_GEOMETRY, saveGeometry());
  settings.setValue(SETTING_WINDOWSTATE, static_cast<int>(windowState()));
  QMainWindow::closeEvent(event);
}

void MainWindow::worldChanged()
{
  if(m_world)
    m_mdiArea->closeAllSubWindows();

  if(m_connection)
    m_world = m_connection->world();
  else
    m_world.clear();

  if(m_world)
  {
    if(AbstractProperty* edit = m_world->getProperty("edit"))
      connect(edit, &AbstractProperty::valueChangedBool, m_actionEdit, &QAction::setChecked);
  }

  updateWindowTitle();
}

void MainWindow::updateWindowTitle()
{
  QString title = "Traintastic v" + QApplication::applicationVersion();
  if(m_world)
    title = m_world->getProperty("name")->toString() + " - " + title;
  setWindowTitle(title);
}

void MainWindow::loadWorld()
{
  if(!m_connection)
    return;

  std::unique_ptr<WorldListDialog> d = std::make_unique<WorldListDialog>(m_connection, this);
  if(d->exec() == QDialog::Accepted)
  {
    Method* method = m_connection->traintastic()->getMethod("load_world");
    if(Q_LIKELY(method))
      method->call(d->uuid());
  }
}

void MainWindow::importWorld()
{
}

void MainWindow::exportWorld()
{
}

void MainWindow::toggleFullScreen()
{
  const bool fullScreen = qobject_cast<QAction*>(sender())->isChecked();
  if(fullScreen == isFullScreen())
    return;

  if(fullScreen)
  {
    m_beforeFullScreenGeometry = saveGeometry();
    showFullScreen();
  }
  else
  {
    showNormal(); // required to exit fullscreen mode
    if(!m_beforeFullScreenGeometry.isEmpty())
      restoreGeometry(m_beforeFullScreenGeometry);
    else
      showMaximized();
  }
}

void MainWindow::toggleConsole()
{
  if(m_serverConsole)
  {
    delete m_serverConsole;
    m_serverConsole = nullptr;
    m_actionServerConsole->setChecked(false);
  }
  else
  {
    m_serverConsole = new ServerConsoleWidget(m_splitter);
    m_splitter->addWidget(m_serverConsole);
    m_actionServerConsole->setChecked(true);
  }
}

/*
void MainWindow::showObjectEdit(const QString& id)
{
  if(!m_mdiSubWindow.objectEdit.contains(id))
  {
    ObjectEditSubWindow* window = new ObjectEditSubWindow(id);
    m_mdiArea->addSubWindow(window);
    window->setAttribute(Qt::WA_DeleteOnClose);
    connect(window, &QMdiSubWindow::destroyed, [this, id](QObject*){ m_mdiSubWindow.objectEdit.remove(id); });
    window->show();
    m_mdiSubWindow.objectEdit[id] = window;
  }
  else
    m_mdiArea->setActiveSubWindow(m_mdiSubWindow.objectEdit[id]);
}

void MainWindow::showObjectEdit(const ObjectPtr& object)
{
  const QString& id = object->getProperty("id")->toString();
  if(!m_mdiSubWindow.objectEdit.contains(id))
  {
    ObjectEditSubWindow* window = new ObjectEditSubWindow(object);
    m_mdiArea->addSubWindow(window);
    window->setAttribute(Qt::WA_DeleteOnClose);
    connect(window, &QMdiSubWindow::destroyed, [this, id](QObject*){ m_mdiSubWindow.objectEdit.remove(id); });
    window->show();
    m_mdiSubWindow.objectEdit[id] = window;
  }
  else
    m_mdiArea->setActiveSubWindow(m_mdiSubWindow.objectEdit[id]);
}
*/
void MainWindow::showObject(const ObjectPtr& object)
{
  const QString& id = object->getProperty("id")->toString();
  if(!m_mdiSubWindows.contains(id))
  {
    QMdiSubWindow* window = new ObjectSubWindow(object);
    m_mdiSubWindows[id] = window;
    m_mdiArea->addSubWindow(window);
    window->setAttribute(Qt::WA_DeleteOnClose);
    connect(window, &QMdiSubWindow::destroyed, this,
      [this, id](QObject*)
      {
        m_mdiSubWindows.remove(id);
      });
    window->show();
  }
  else
    m_mdiArea->setActiveSubWindow(m_mdiSubWindows[id]);
}

void MainWindow::showObject(const QString& id, const QString& title)
{
  if(!m_mdiSubWindows.contains(id))
  {
    QMdiSubWindow* window = new ObjectSubWindow(m_connection, id);
    if(!title.isEmpty())
      window->setWindowTitle(title);
    m_mdiSubWindows[id] = window;
    m_mdiArea->addSubWindow(window);
    window->setAttribute(Qt::WA_DeleteOnClose);
    connect(window, &QMdiSubWindow::destroyed, this,
      [this, id](QObject*)
      {
        m_mdiSubWindows.remove(id);
      });
    window->show();
  }
  else
    m_mdiArea->setActiveSubWindow(m_mdiSubWindows[id]);
}

void MainWindow::showAbout()
{
  QMessageBox::about(this, tr("About Traintastic"),
    "<h2>Traintastic v" + QApplication::applicationVersion() + " <small>" TRAINTASTIC_CODENAME "</small></h2>"
    "<p>Copyright &copy; 2019-2020 Reinder Feenstra</p>"
    "<p>This program is free software; you can redistribute it and/or"
    " modify it under the terms of the GNU General Public License"
    " as published by the Free Software Foundation; either version 2"
    " of the License, or (at your option) any later version.</p>"
    "<p>This program is distributed in the hope that it will be useful,"
    " but WITHOUT ANY WARRANTY; without even the implied warranty of"
    " MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the"
    " GNU General Public License for more details.</p>");
}

void MainWindow::connectionStateChanged()
{
  const bool connected = m_connection && m_connection->state() == Connection::State::Connected;

  m_mdiArea->setEnabled(connected);

  if(connected && m_actionServerConsole->isChecked() && !m_serverConsole)
  {
    m_serverConsole = new ServerConsoleWidget(m_splitter);
    m_splitter->addWidget(m_serverConsole);
  }

  if(m_connection && m_connection->state() == Connection::State::Disconnected)
  {
    m_connection.clear();
    if(m_serverConsole)
    {
      delete m_serverConsole;
      m_serverConsole = nullptr;
    }
    worldChanged();
  }

  updateActions();

  //if(client.state() == Client::State::SocketError)
  //  statusBar()->showMessage(client.errorString());
}

void MainWindow::updateActions()
{
  const bool connected = m_connection && m_connection->state() == Connection::State::Connected;
  const bool haveWorld = connected && m_connection->world();


  m_actionConnectToServer->setEnabled(!m_connection);
  m_actionConnectToServer->setVisible(!connected);
  m_actionDisconnectFromServer->setVisible(connected);
  m_actionNewWorld->setEnabled(connected);
  m_actionLoadWorld->setEnabled(connected);
  m_actionSaveWorld->setEnabled(haveWorld);
  m_actionImportWorld->setEnabled(haveWorld    && false);
  m_actionExportWorld->setEnabled(haveWorld    && false);

  m_actionServerConsole->setEnabled(connected);
  m_menuServer->setEnabled(haveWorld);
  m_actionServerSettings->setEnabled(connected);
  if(connected)
  {
    Method* m;
    m = m_connection->traintastic()->getMethod("restart");
    m_actionServerRestart->setEnabled(m && m->getAttributeBool(AttributeName::Enabled, false));
    m = m_connection->traintastic()->getMethod("shutdown");
    m_actionServerShutdown->setEnabled(m && m->getAttributeBool(AttributeName::Enabled, false));
  }

  m_actionTrackPowerOff->setEnabled(haveWorld);
  m_actionTrackPowerOn->setEnabled(haveWorld);
  m_actionEmergencyStop->setEnabled(haveWorld);
  m_actionEdit->setEnabled(haveWorld);
  m_actionEdit->setChecked(haveWorld && m_connection->world()->getProperty("edit")->toBool());

  setMenuEnabled(m_menuObjects, haveWorld);
  if(haveWorld && !m_connection->world()->hasProperty("lua_scripts"))
    m_actionLuaScript->setEnabled(false);

  //m_menuHardware->setEnabled(haveWorld);
  //m_action
  //m_actionLua->setEnabled(haveWorld);

  if(connected && !haveWorld)
  {
    m_mdiArea->addBackgroundAction(m_actionNewWorld);
    m_mdiArea->addBackgroundAction(m_actionLoadWorld);
  }
  else
  {
    m_mdiArea->removeBackgroundAction(m_actionNewWorld);
    m_mdiArea->removeBackgroundAction(m_actionLoadWorld);
  }


  //updateModeActions();
}

//void MainWindow::updateModeActions()
/*
  const bool haveWorld = m_connection && m_connection->state() == Connection::State::Connected && m_connection->world();
  const TraintasticMode mode = /*haveWorld ? static_cast<TraintasticMode>(m_connection->traintastic()->getProperty("mode")->toInt64()) :*//* TraintasticMode::Stop;

  //m_actionModeRun->setEnabled(haveWorld && mode != TraintasticMode::Edit);
  //m_actionModeStop->setEnabled(haveWorld);
  //m_actionModeEdit->setEnabled(haveWorld && mode != TraintasticMode::Run);

  switch(mode)
  {
    case TraintasticMode::Run:
      m_actionModeRun->setChecked(true);
      break;

    case TraintasticMode::Stop:
      m_actionModeStop->setChecked(true);
      break;

    case TraintasticMode::Edit:
      m_actionModeEdit->setChecked(true);
      break;

    default:
      Q_ASSERT(false);
      break;
  }*/

