/**
 * client/src/mainwindow/mainwindowstatusbar.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#include "mainwindowstatusbar.hpp"
#include <QLabel>
#include "../mainwindow.hpp"
#include "../network/object.hpp"
#include "../network/abstractproperty.hpp"
#include "../settings/statusbarsettings.hpp"

MainWindowStatusBar::MainWindowStatusBar(MainWindow& mainWindow)
  : QStatusBar(&mainWindow)
  , m_mainWindow{mainWindow}
  , m_clockLabel{new QLabel(this)}
{
  // clock:
  m_clockLabel->setMinimumWidth(m_clockLabel->fontMetrics().averageCharWidth() * 5);
  m_clockLabel->setAlignment(Qt::AlignRight);
  addPermanentWidget(m_clockLabel);
  connect(&m_mainWindow, &MainWindow::worldClockChanged,
    [this]()
    {
      if(const auto& clock = m_mainWindow.worldClock())
        if(const auto* minute = clock->getProperty("minute"))
          connect(minute, &AbstractProperty::valueChanged, this, &MainWindowStatusBar::updateClock);
    });

  // settings:
  connect(&StatusBarSettings::instance(), &StatusBarSettings::changed, this, &MainWindowStatusBar::settingsChanged);
  settingsChanged();
}

void MainWindowStatusBar::settingsChanged()
{
  clockChanged();
}

void MainWindowStatusBar::clockChanged()
{
  const bool visible = StatusBarSettings::instance().showClock.value();

  if(m_clockLabel->isVisible() == visible)
    return;

  if(visible)
    updateClock();

  m_clockLabel->setVisible(visible);
}

void MainWindowStatusBar::updateClock()
{
  if(const auto& clock = m_mainWindow.worldClock())
    m_clockLabel->setText(QString("%1:%2").arg(clock->getPropertyValueInt("hour", 0)).arg(clock->getPropertyValueInt("minute", 0), 2, 10, QChar('0')));
  else
    m_clockLabel->setText("--:--");
}
