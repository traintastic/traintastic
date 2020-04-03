/**
 * client/src/mainwindow.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019 Reinder Feenstra
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
#include "mdiarea.hpp"
#include "dialog/connectdialog.hpp"
#include "network/connection.hpp"
#include "network/object.hpp"
#include "network/property.hpp"
#include "network/method.hpp"
//#include "subwindow/objecteditsubwindow.hpp"
#include "subwindow/objectsubwindow.hpp"


#include <QDesktopServices>
#include <locale/locale.hpp>

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
  m_mdiArea{new MdiArea()}
{
  instance = this;

  setWindowTitle("Traintastic v" + QApplication::applicationVersion());

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
    m_actionLoadWorld = menu->addAction(QIcon(":/dark/world_load.svg"), Locale::tr("qtapp.mainmenu:load_world") + "...", this, &MainWindow::loadWorld);
    m_actionSaveWorld = menu->addAction(QIcon(":/dark/world_save.svg"), Locale::tr("qtapp.mainmenu:save_world"),
      [this]()
      {
        if(m_world)
          if(Method* method = m_world->getMethod("save"))
            method->call();
      });
    menu->addSeparator();
    m_actionImportWorld = menu->addAction(QIcon(":/dark/world_import.svg"), Locale::tr("qtapp.mainmenu:import_world") + "...", this, &MainWindow::importWorld);
    m_actionExportWorld = menu->addAction(QIcon(":/dark/world_export.svg"), Locale::tr("qtapp.mainmenu:export_world") + "...", this, &MainWindow::exportWorld);
    menu->addSeparator();
    menu->addAction(Locale::tr("qtapp.mainmenu:quit"), this, &MainWindow::close);

    menu = menuBar()->addMenu(Locale::tr("qtapp.mainmenu:view"));
    actFullScreen = menu->addAction(Locale::tr("qtapp.mainmenu:fullscreen"), this, &MainWindow::toggleFullScreen);
    actFullScreen->setCheckable(true);
    actFullScreen->setShortcut(Qt::Key_F11);

    m_menuObjects = menuBar()->addMenu(Locale::tr("qtapp.mainmenu:objects"));
    menu = m_menuObjects->addMenu(QIcon(":/dark/hardware.svg"), Locale::tr("qtapp.mainmenu:hardware"));
    menu->addAction(Locale::tr("world:command_stations") + "...", [this](){ showObject("world.command_stations"); });
    menu->addAction(Locale::tr("world:decoders") + "...", [this](){ showObject("world.decoders"); });
    menu->addAction(Locale::tr("world:inputs") + "...", [this](){ showObject("world.inputs"); });
    m_menuObjects->addAction(Locale::tr("world:clock") + "...", [this](){ showObject("world.clock"); });
    m_menuObjects->addAction(QIcon(":/dark/lua.svg"), Locale::tr("world:lua_scripts") + "...", [this](){ showObject("world.lua_scripts"); });

    menu = menuBar()->addMenu(Locale::tr("qtapp.mainmenu:tools"));
    menu->addAction(Locale::tr("qtapp.mainmenu:settings") + "...")->setEnabled(false);
    menu->addSeparator();
    m_actionServerSettings = menu->addAction(Locale::tr("qtapp.mainmenu:server_settings") + "...", this, [this](){ showObject("traintastic.settings"); });
    m_actionServerConsole = menu->addAction(Locale::tr("qtapp.mainmenu:server_console") + "...", this, [this](){ showObject("traintastic.console"); });
    m_actionServerConsole->setShortcut(Qt::Key_F12);

    menu = menuBar()->addMenu(Locale::tr("qtapp.mainmenu:help"));
    menu->addAction(Locale::tr("qtapp.mainmenu:help"), [](){ QDesktopServices::openUrl("https://traintastic.org/manual?version=" + QApplication::applicationVersion()); })->setShortcut(Qt::Key_F1);
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

  setCentralWidget(w);
  setStatusBar(new QStatusBar());

  QSettings settings;
  if(settings.contains(SETTING_GEOMETRY))
    restoreGeometry(settings.value(SETTING_GEOMETRY).toByteArray());
  if(settings.contains(SETTING_WINDOWSTATE))
    setWindowState(static_cast<Qt::WindowState>(settings.value(SETTING_WINDOWSTATE).toInt()));
  actFullScreen->setChecked(isFullScreen());

  //connect(Client::instance, &Client::stateChanged, this, &MainWindow::clientStateChanged);

  clientStateChanged();
}

MainWindow::~MainWindow()
{
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
        m_world = m_connection->world();
        updateActions();
      });
    clientStateChanged();
  }
}

void MainWindow::disconnectFromServer()
{
  if(!m_connection)
    return;

  m_world.clear();
  m_connection->disconnectFromHost();
  m_connection.clear();
  clientStateChanged();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
  QSettings settings;
  settings.setValue(SETTING_GEOMETRY, saveGeometry());
  settings.setValue(SETTING_WINDOWSTATE, static_cast<int>(windowState()));
  QMainWindow::closeEvent(event);
}

/*void MainWindow::setMode(TraintasticMode value)
{
  if(!m_connection)
    return;

  if(m_connection->state() == Connection::State::Connected && m_connection->traintastic())
    m_connection->traintastic()->getProperty("mode")->setValueInt64(static_cast<int64_t>(value));
  else
    updateModeActions();
}*/

void MainWindow::loadWorld()
{
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
    connect(window, &QMdiSubWindow::destroyed, [this, id](QObject*){ m_mdiSubWindows.remove(id); });
    window->show();
  }
  else
    m_mdiArea->setActiveSubWindow(m_mdiSubWindows[id]);
}

void MainWindow::showObject(const QString& id)
{
  if(!m_mdiSubWindows.contains(id))
  {
    QMdiSubWindow* window = new ObjectSubWindow(m_connection, id);
    m_mdiSubWindows[id] = window;
    m_mdiArea->addSubWindow(window);
    window->setAttribute(Qt::WA_DeleteOnClose);
    connect(window, &QMdiSubWindow::destroyed, [this, id](QObject*){ m_mdiSubWindows.remove(id); });
    window->show();
  }
  else
    m_mdiArea->setActiveSubWindow(m_mdiSubWindows[id]);
}

void MainWindow::showAbout()
{
  QMessageBox::about(this, tr("About Traintastic"),
    "<h2>Traintastic <small>v" + QApplication::applicationVersion() + "</small></h2>"
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

void MainWindow::clientStateChanged()
{
  const bool connected = m_connection && m_connection->state() == Connection::State::Connected;

  m_mdiArea->setEnabled(connected);

  //if(connected)
  //  connect(m_connection->traintastic()->getProperty("mode"), &Property::valueChanged, this, &MainWindow::updateModeActions);

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
  m_actionImportWorld->setEnabled(haveWorld);
  m_actionExportWorld->setEnabled(haveWorld);

  m_actionServerSettings->setEnabled(connected);
  m_actionServerConsole->setEnabled(connected);

  m_actionTrackPowerOff->setEnabled(haveWorld);
  m_actionTrackPowerOn->setEnabled(haveWorld);
  m_actionEmergencyStop->setEnabled(haveWorld);
  m_actionEdit->setEnabled(haveWorld);
  if(!haveWorld)
    m_actionEdit->setChecked(false);

  setMenuEnabled(m_menuObjects, haveWorld);

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

