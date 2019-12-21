/**
 * client/src/mainwindow.hpp
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

#ifndef CLIENT_MAINWINDOW_HPP
#define CLIENT_MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QMap>
#include <enum/traintasticmode.hpp>

class QMdiArea;
class QActionGroup;
class ObjectEditSubWindow;
class HardwareListSubWindow;
class ServerSettingsSubWindow;
class ServerConsoleSubWindow;

class MainWindow : public QMainWindow
{
  Q_OBJECT

  protected:
    QMdiArea* m_mdiArea;
    struct
    {
      QMap<QString, ObjectEditSubWindow*> objectEdit;
      HardwareListSubWindow* hardwareList = nullptr;
      ServerSettingsSubWindow* serverSettings = nullptr;
      ServerConsoleSubWindow* serverConsole = nullptr;
    } m_mdiSubWindow;
    QAction* m_actionConnectToServer;
    QAction* m_actionDisconnectFromServer;
    QAction* m_actionNewWorld;
    QAction* m_actionLoadWorld;
    QAction* m_actionSaveWorld;
    QAction* m_actionImportWorld;
    QAction* m_actionExportWorld;
    QAction* m_actionHardware;
    QAction* m_actionServerSettings;
    QAction* m_actionServerConsole;
    QActionGroup* m_actionGroupMode;
    QAction* m_actionModeRun;
    QAction* m_actionModeStop;
    QAction* m_actionModeEdit;
    QByteArray m_beforeFullScreenGeometry;

    void closeEvent(QCloseEvent* event) final;
    void setMode(TraintasticMode value);

  protected slots:
    void disconnectFromServer();
    void newWorld();
    void loadWorld();
    void saveWorld();
    void importWorld();
    void exportWorld();
    void toggleFullScreen();
    void showHardware();

    void showServerSettings();
    void showServerConsole();
    void showAbout();
    void clientStateChanged();
    void updateModeActions();

  public:
    inline static MainWindow* instance = nullptr;

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

  public slots:
    void connectToServer();
    void showObjectEdit(const QString& id);
};

#endif
