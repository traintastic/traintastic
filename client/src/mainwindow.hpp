/**
 * client/src/mainwindow.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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

class MdiArea;
class QSplitter;
class QMdiSubWindow;
class QActionGroup;
class QToolButton;
class Connection;
class AbstractProperty;
class ObjectEditSubWindow;
class HardwareListSubWindow;
class LuaScriptsSubWindow;
class ServerSettingsSubWindow;
class ServerConsoleWidget;

class MainWindow : public QMainWindow
{
  Q_OBJECT

  protected:
    std::shared_ptr<Connection> m_connection;
    ObjectPtr m_world;
    QSplitter* m_splitter;
    MdiArea* m_mdiArea;
    ServerConsoleWidget* m_serverConsole;
    QMap<QString, QMdiSubWindow*> m_mdiSubWindows;
    QAction* m_actionConnectToServer;
    QAction* m_actionDisconnectFromServer;
    QAction* m_actionNewWorld;
    QAction* m_actionLoadWorld;
    QAction* m_actionSaveWorld;
    QAction* m_actionImportWorld;
    QAction* m_actionExportWorld;
    QMenu* m_menuObjects;
    QAction* m_actionLuaScript;
    QMenu* m_menuServer;
    QAction* m_actionServerSettings;
    QAction* m_actionServerRestart;
    QAction* m_actionServerShutdown;
    QAction* m_actionServerConsole;
    // Main toolbar:
    QToolButton* m_worldOnlineOfflineToolButton;
    QAction* m_worldOnlineAction;
    QAction* m_worldOfflineAction;
    QToolButton* m_worldPowerOnOffToolButton;
    QAction* m_worldPowerOnAction;
    QAction* m_worldPowerOffAction;
    QAction* m_worldStopAction;
    QAction* m_worldRunAction;
    QAction* m_worldMuteAction;
    QAction* m_worldEditAction;

    QByteArray m_beforeFullScreenGeometry;

    void closeEvent(QCloseEvent* event) final;
    void worldChanged();
    void updateWindowTitle();

  protected slots:
    void disconnectFromServer();
    void loadWorld();
    void importWorld();
    void exportWorld();
    void toggleFullScreen();
    void toggleConsole();
    void showAbout();
    void connectionStateChanged();
    void updateActions();
    void worldStateChanged(int64_t value);
    //void updateModeActions();

  public:
    inline static MainWindow* instance = nullptr;

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() final;

    const std::shared_ptr<Connection>& connection() { return m_connection; }

  public slots:
    void connectToServer();
    void showObject(const ObjectPtr& object);
    void showObject(const QString& id, const QString& title = "");
};

#endif
