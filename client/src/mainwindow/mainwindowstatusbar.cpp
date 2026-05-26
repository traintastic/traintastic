/**
 * client/src/mainwindow/mainwindowstatusbar.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2024 Reinder Feenstra
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
#include <QHBoxLayout>
#include <QLabel>
#include "../mainwindow.hpp"
#include "../network/connection.hpp"
#include "../network/object.hpp"
#include "../network/abstractproperty.hpp"
#include "../network/objectvectorproperty.hpp"
#include "../network/error.hpp"
#include "../settings/statusbarsettings.hpp"
#include "../widget/status/interfacestatuswidget.hpp"
#include "../widget/status/luastatuswidget.hpp"
#include "../widget/status/simulationstatuswidget.hpp"

MainWindowStatusBar::MainWindowStatusBar(MainWindow& mainWindow)
  : QStatusBar(&mainWindow)
  , m_mainWindow{mainWindow}
  , m_statuses{new QWidget(this)}
  , m_clockLabel{new QLabel(this)}
  , m_statusesRequest{Connection::invalidRequestId}
{
  // statuses:
  m_statuses->setLayout(new QHBoxLayout());
  m_statuses->layout()->setContentsMargins(0, 0, 0, 0);
  addPermanentWidget(m_statuses);

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

void MainWindowStatusBar::worldChanged()
{
  if(const auto world = m_mainWindow.world())
  {
    if(auto* statuses = dynamic_cast<ObjectVectorProperty*>(world->getVectorProperty("statuses")))
    {
      connect(statuses, &ObjectVectorProperty::valueChanged, this, &MainWindowStatusBar::updateStatuses);
      updateStatuses();
    }
  }
  else // no world
  {
    clearStatuses();
    updateClock();
  }
}

void MainWindowStatusBar::settingsChanged()
{
  const auto& settings = StatusBarSettings::instance();
  if(settings.showStatuses.value() != m_statuses->isVisible())
    updateStatuses();
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

void MainWindowStatusBar::clearStatuses()
{
  auto* l = m_statuses->layout();
  while(!l->isEmpty())
    l->removeItem(l->itemAt(0));
}

void MainWindowStatusBar::updateStatuses()
{
  if(!m_mainWindow.world())
    return;

  m_statuses->setVisible(StatusBarSettings::instance().showStatuses.value());
  if(!m_statuses->isVisible())
  {
    clearStatuses();
    return;
  }

  if(auto* statuses = dynamic_cast<ObjectVectorProperty*>(m_mainWindow.world()->getVectorProperty("statuses")))
  {
    m_mainWindow.connection()->cancelRequest(m_statusesRequest);

    if(statuses->empty())
    {
      clearStatuses();
      return;
    }

    m_statusesRequest = statuses->getObjects(0, statuses->size() - 1,
      [this](const std::vector<ObjectPtr>& objects, std::optional<const Error> error)
      {
        m_statusesRequest = Connection::invalidRequestId;
        clearStatuses();
        if(error || objects.empty())
          return;

        for(const auto& object : objects)
        {
          if(object->classId() == "status.interface")
            m_statuses->layout()->addWidget(new InterfaceStatusWidget(object, this));
          else if(object->classId() == "status.lua")
            m_statuses->layout()->addWidget(new LuaStatusWidget(object, this));
          else if(object->classId() == "status.simulation")
            m_statuses->layout()->addWidget(new SimulationStatusWidget(object, this));
        }
      });
  }
}
