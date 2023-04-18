/**
 * client/src/mainwindow/mainwindowstatusbar.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_MAINWINDOW_MAINWINDOWSTATUSBAR_HPP
#define TRAINTASTIC_CLIENT_MAINWINDOW_MAINWINDOWSTATUSBAR_HPP

#include <QStatusBar>

class MainWindow;
class QLabel;

class MainWindowStatusBar : public QStatusBar
{
  private:
    MainWindow& m_mainWindow;
    QWidget* m_statuses;
    QLabel* m_clockLabel;
    int m_statusesRequest;

    void settingsChanged();

    void clockChanged();
    void updateClock();

    void clearStatuses();
    void updateStatuses();

  public:
    MainWindowStatusBar(MainWindow& mainWindow);

    void worldChanged();
};

#endif
