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

  m_progressBar->setRange(static_cast<int>(Step::Start), static_cast<uint>(Step::Done));

  m_label->setAlignment(Qt::AlignCenter);

  connect(m_start, &QPushButton::clicked,
    [this]()
    {
      QSettings settings;
      m_outputDir.setPath(QFileDialog::getExistingDirectory(this, "Select output directory", settings.value("ScreenshotCreator/OutputDirectory", QDir::homePath()).toString()));
      if(m_outputDir.exists())
      {
        m_start->setEnabled(false);
        settings.setValue("ScreenshotCreator/OutputDirectory", m_outputDir.path());
        using namespace std::chrono_literals;
        m_stepTimer = startTimer(500ms);
      }
    });

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
    step();
  }
}

void ScreenShotDialog::step()
{
  while(!m_dialogsToClose.isEmpty())
  {
    m_dialogsToClose.pop()->close();
  }

  switch(m_step)
  {
    using enum Step;

    case Start:
      next();
      return;

    case ConnectToServer:
      if(m_mainWindow.connection())
      {
        next();
      }
      return;

    case CloseWorld:
      if(!m_mainWindow.world())
      {
        next();
      }
      else
      {
        m_mainWindow.m_actionCloseWorld->trigger();
      }
      return;

    case NewWorld:
      if(m_mainWindow.world())
      {
        next();
      }
      else
      {
        saveMainWindowImage(QStringLiteral("getting-started/traintastic-startup-no-world.png"));
        m_mainWindow.m_actionNewWorld->trigger();
      }
      return;

    case NewWorldWizard:
      if(m_mainWindow.m_wizard.newWorld)
      {
        m_mainWindow.m_wizard.newWorld->next();
        next();
      }
      return;

    case NewWorldWizardSetWorldName:
      m_mainWindow.m_wizard.newWorld->button(QWizard::NextButton)->setFocus();
      m_mainWindow.m_world->setPropertyValue("name", QStringLiteral("My First World"));
      next();
      return;

    case NewWorldWizardSetWorldNameShoot:
      saveWidgetImage(m_mainWindow.m_wizard.newWorld.get(), QStringLiteral("getting-started/new-world-wizard-set-name.png"));
      m_mainWindow.m_wizard.newWorld->next();
      next();
      return;

    case NewWorldWizardSetWorldScale:
      saveWidgetImage(m_mainWindow.m_wizard.newWorld.get(), QStringLiteral("getting-started/new-world-wizard-select-scale.png"));
      m_mainWindow.m_wizard.newWorld->next();
      next();
      return;

    case NewWorldWizardFinish:
      saveWidgetImage(m_mainWindow.m_wizard.newWorld.get(), QStringLiteral("getting-started/new-world-wizard-finish.png"));
      m_mainWindow.m_wizard.newWorld->accept();
      next();
      return;

    case InterfaceListEmpty:
      if(auto it = m_mainWindow.m_subWindows.find(SubWindow::windowId(SubWindowType::Object, "world.interfaces")); it != m_mainWindow.m_subWindows.end())
      {
        auto* w = it.value();
        w->move(10, 10);
        w->resize(400, 300);
        saveWidgetImage(w, QStringLiteral("interface/interface-list-empty.png"));
        w->close();
        next();
      }
      else
      {
        m_mainWindow.m_menuObjects->actions()[0]->menu()->actions()[0]->trigger(); // Object -> Hardware -> Interfaces
      }
      return;

    case LuaScriptListEmpty:
      if(auto it = m_mainWindow.m_subWindows.find(SubWindow::windowId(SubWindowType::Object, "world.lua_scripts")); it != m_mainWindow.m_subWindows.end())
      {
        auto* w = it.value();
        w->move(10, 10);
        w->resize(400, 300);
        saveWidgetImage(w, QStringLiteral("lua/lua-script-list-empty.png"));
        static_cast<ObjectListWidget*>(w->widget())->m_actionCreate->trigger();
        next();
      }
      else
      {
        m_mainWindow.m_menuObjects->actions()[5]->trigger(); // Object -> Lua scripts
      }
      return;

    case LuaScriptEditor:
      if(auto it = m_mainWindow.m_subWindows.find(SubWindow::windowId(SubWindowType::Object, "script_1")); it != m_mainWindow.m_subWindows.end())
      {
        auto* w = it.value();
        w->move(420, 10);
        w->resize(400, 300);
        saveWidgetImage(w, QStringLiteral("lua/lua-script-editor.png"));
        m_mainWindow.m_mdiArea->closeAllSubWindows();
        next();
      }
      return;

    case Done:
      killTimer(m_stepTimer);
      m_stepTimer = 0;
      return;
  }
}

void ScreenShotDialog::next()
{
  if(m_step < Step::Done) [[likely]]
  {
    m_step = static_cast<Step>(static_cast<uint>(m_step) + 1);
  }
  m_progressBar->setValue(static_cast<int>(m_step));
  m_label->setText(getStepLabel(m_step));
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

void ScreenShotDialog::saveMainWindowImage(const QString& filename)
{
  savePixmap(m_mainWindow.screen()->grabWindow(0).copy(m_mainWindow.frameGeometry()), filename);
}

QString ScreenShotDialog::getStepLabel(Step step)
{
  switch(step)
  {
    using enum Step;

    case Start:
      return "Start";
    case ConnectToServer:
      return "ConnectToServer";
    case CloseWorld:
      return "CloseWorld";
    case NewWorld:
      return "NewWorld";
    case NewWorldWizard:
      return "NewWorldWizard";
    case NewWorldWizardSetWorldName:
      return "NewWorldWizardSetWorldName";
    case NewWorldWizardSetWorldScale:
      return "NewWorldWizardSetWorldScale";
    case NewWorldWizardFinish:
      return "NewWorldWizardFinish";
    case InterfaceListEmpty:
      return "InterfaceListEmpty";
    case LuaScriptListEmpty:
      return "LuaScriptListEmpty";
    case LuaScriptEditor:
      return "LuaScriptEditor";
    case Done:
      return "Done";
  }
  //assert(false);
  return QString("Step %1").arg(static_cast<uint>(step));
}
