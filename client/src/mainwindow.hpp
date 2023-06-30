/**
 * client/src/mainwindow.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_MAINWINDOW_HPP
#define TRAINTASTIC_CLIENT_MAINWINDOW_HPP

#include <QMainWindow>
#include <QMap>
#include "network/objectptr.hpp"
#include "subwindow/subwindowtype.hpp"

class MdiArea;
class MainWindowStatusBar;
class QMdiSubWindow;
class QSplitter;
class QActionGroup;
class QToolButton;
class SubWindow;
class Connection;
class AbstractProperty;
class ObjectEditSubWindow;
class HardwareListSubWindow;
class LuaScriptsSubWindow;
class ServerSettingsSubWindow;
class ServerLogWidget;

class MainWindow : public QMainWindow
{
  Q_OBJECT

  protected:
    std::shared_ptr<Connection> m_connection;
    ObjectPtr m_world;
    int m_clockRequest;
    ObjectPtr m_clock;
    QSplitter* m_splitter;
    MdiArea* m_mdiArea;
    MainWindowStatusBar* m_statusBar;
    ServerLogWidget* m_serverLog;
    QMdiSubWindow* m_clockWindow = nullptr;
    QMap<QString, SubWindow*> m_subWindows;
    // Main menu:
    QAction* m_actionConnectToServer;
    QAction* m_actionDisconnectFromServer;
    QAction* m_actionNewWorld;
    QAction* m_actionLoadWorld;
    QAction* m_actionSaveWorld;
    QAction* m_actionCloseWorld;
    QAction* m_actionImportWorld;
    QAction* m_actionExportWorld;
    QMenu* m_menuWorld;
    QMenu* m_menuConnection;
    QAction* m_worldOnlineAction;
    QAction* m_worldOfflineAction;
    QMenu* m_menuPower;
    QAction* m_worldPowerOnAction;
    QAction* m_worldPowerOffAction;
    QAction* m_worldStopAction;
    QAction* m_worldRunAction;
    QAction* m_clockAction;
    QAction* m_worldMuteMenuAction;
    QAction* m_worldNoSmokeMenuAction;
    QAction* m_worldEditAction;
    QAction* m_worldSimulationAction;
    QMenu* m_menuObjects;
    QAction* m_actionLuaScript;
    QAction* m_actionFullScreen;
    QAction* m_actionViewToolbar;
    QAction* m_actionClock;
    QMenu* m_menuServer;
    QAction* m_actionServerSettings;
    QAction* m_actionServerRestart;
    QAction* m_actionServerShutdown;
    QAction* m_actionServerLog;
    QMenu* m_menuProgramming;
    // Main toolbar:
    QToolBar* m_toolbar;
    QToolButton* m_worldOnlineOfflineToolButton;
    QToolButton* m_worldPowerOnOffToolButton;
    QAction* m_worldMuteToolbarAction;
    QAction* m_worldNoSmokeToolbarAction;

    QByteArray m_beforeFullScreenGeometry;

    void closeEvent(QCloseEvent* event) final;
    void changeEvent(QEvent* event) final;
    void worldChanged();
    void updateWindowTitle();
    void addSubWindow(const QString& windowId, SubWindow* window);

  protected slots:
    void disconnectFromServer();
    void loadWorld();
    void toggleFullScreen();
    void viewClockWindow(bool value);
    void toggleServerLog();
    void showAbout();
    void connectionStateChanged();
    void updateActions();
    void worldStateChanged(int64_t value);

  public:
    inline static MainWindow* instance = nullptr;

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() final;

    const std::shared_ptr<Connection>& connection() { return m_connection; }

    const ObjectPtr& world() const;
    const ObjectPtr& worldClock() const { return m_clock; }

  public slots:
    void connectToServer(const QString& url = QString());
    void showObject(const ObjectPtr& object, SubWindowType flags = SubWindowType::Object);
    void showObject(const QString& id, const QString& title = "", SubWindowType flags = SubWindowType::Object);

  signals:
    void worldClockChanged();
};

#endif
