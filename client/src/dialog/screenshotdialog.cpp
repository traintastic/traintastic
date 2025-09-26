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
#include <QLineEdit>
#include <QFileDialog>
#include <QSettings>
#include <QScreen>
#include "../mainwindow.hpp"
#include "../mdiarea.hpp"
#include "../network/method.hpp"
#include "../network/object.hpp"
#include "../settings/generalsettings.hpp"
#include "../subwindow/subwindow.hpp"
#include "../widget/object/abstracteditwidget.hpp"
#include "../widget/objectlist/objectlistwidget.hpp"
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
        w->close();
        return true;
      }
      return false;
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
