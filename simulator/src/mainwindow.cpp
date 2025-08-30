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

#include "mainwindow.hpp"
#include <QFileDialog>
#include <QMenuBar>
#include <QToolBar>
#include <QMessageBox>
#include <QSettings>
#include <QFileInfo>
#include <QStatusBar>
#include <QLabel>
#include <QKeyEvent>
#include "simulatorview.hpp"
#include <version.hpp>
#include <traintastic/copyright.hpp>
#include <traintastic/utils/standardpaths.hpp>

namespace
{

QString formatDuration(float value)
{
  if(value < 1e-3f)
  {
    return QString(u8"%1 \u00B5s").arg(value * 1e6f, 0, 'f', 0);
  }
  else if(value < 1e-2f)
  {
    return QString("%1 ms").arg(value * 1e3f, 0, 'f', 2);
  }
  else if(value < 1e-1f)
  {
    return QString("%1 ms").arg(value * 1e3f, 0, 'f', 1);
  }
  return QString("%1 ms").arg(value * 1e3f, 0, 'f', 0);
}

float mean(const std::vector<float>& values)
{
  return std::accumulate(values.begin(), values.end(), 0.0f) / values.size();
}

}

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags)
  : QMainWindow(parent, flags)
  , m_view{new SimulatorView(this)}
  , m_tickActive{new QLabel(this)}
{
  setWindowIcon(QIcon(":/appicon.svg"));
  setWindowTitle("Traintastic simulator v" TRAINTASTIC_VERSION_FULL);

  resize(800, 600);

  setCentralWidget(m_view);

  // Menu bar:
  {
    QMenu* menu;
    QAction* act;

    setMenuBar(new QMenuBar(this));

    menu = menuBar()->addMenu("File");
    menu->addAction("Load",
      [this]()
      {
        QSettings settings;
        const QString dir = settings.value("LastLoadDir", QString::fromStdString(getSimulatorLayoutPath().string())).toString();
        const auto filename = QFileDialog::getOpenFileName(this, "Load simulation", dir, "*.json");
        if(QFile::exists(filename))
        {
          settings.setValue("LastLoadDir", QFileInfo(filename).absoluteFilePath());
          load(filename);
        }
      });
    menu->addAction("Quit", this, &MainWindow::close);

    menu = menuBar()->addMenu("View");
    m_actFullScreen = menu->addAction("Fullscreen", this, &MainWindow::toggleFullScreen);
    m_actFullScreen->setCheckable(true);
    m_actFullScreen->setShortcut(Qt::Key_F11);
    act = menu->addAction("Show track occupancy");
    act->setCheckable(true);
    act->setChecked(m_view->showTrackOccupancy());
    connect(act, &QAction::toggled, m_view, &SimulatorView::setShowTrackOccupancy);

    menu = menuBar()->addMenu("Help");
    menu->addAction("About...", this, &MainWindow::showAbout);
  }

  // Toolbar:
  {
    auto* toolbar = new QToolBar();
    toolbar->setFloatable(false);
    toolbar->setMovable(false);

    m_power = toolbar->addAction("Power");
    m_power->setCheckable(true);
    connect(m_power,
      &QAction::toggled,
      [this](bool value)
      {
        if(auto* simulator = m_view->simulator())
        {
          simulator->setPowerOn(value);
        }
      });

    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolbar->addWidget(spacer);

    toolbar->addAction("Zoom in", m_view, &SimulatorView::zoomIn);
    toolbar->addAction("Zoom out", m_view, &SimulatorView::zoomOut);
    toolbar->addAction("Zoom fit", m_view, &SimulatorView::zoomToFit);

    addToolBar(Qt::TopToolBarArea, toolbar);
  }

  // Status bar:
  {
    auto* statusBar = new QStatusBar(this);
    statusBar->addPermanentWidget(m_tickActive);
    setStatusBar(statusBar);
  }

  connect(m_view, &SimulatorView::tickActiveChanged,
    [this](float value)
    {
      m_tickActiveFilter.push_back(value);
      if(m_tickActiveFilter.size() >= 15)
      {
        m_tickActive->setText(formatDuration(mean(m_tickActiveFilter)));
        m_tickActiveFilter.clear();
      }
    });
  connect(m_view, &SimulatorView::powerOnChanged, m_power, &QAction::setChecked);
}

void MainWindow::load(const QString& filename)
{
  QFile file(filename);
  if(file.open(QIODeviceBase::ReadOnly))
  {
    m_view->setSimulator(std::make_shared<Simulator>(nlohmann::json::parse(file.readAll().toStdString(),
                                                                           nullptr,
                                                                           true, true)));
    QMetaObject::invokeMethod(m_view, &SimulatorView::zoomToFit, Qt::QueuedConnection);
  }
}

void MainWindow::setFullScreen(bool value)
{
  m_actFullScreen->setChecked(value);
  toggleFullScreen();
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
  if(event->key() == Qt::Key_F11) // Once fullscreen the QAction does't receive the key press because it is hidden.
  {
    m_actFullScreen->setChecked(!m_actFullScreen->isChecked());
    toggleFullScreen();
  }
  else
  {
    QMainWindow::keyPressEvent(event);
  }
}

void MainWindow::toggleFullScreen()
{
  if(m_actFullScreen->isChecked() == isFullScreen())
    return;

  if(m_actFullScreen->isChecked())
  {
    m_beforeFullScreenGeometry = saveGeometry();
    showFullScreen();
    menuBar()->hide();
    for (auto* toolbar : findChildren<QToolBar*>())
    {
      toolbar->hide();
    }
    statusBar()->hide();
  }
  else
  {
    showNormal();
    if(!m_beforeFullScreenGeometry.isEmpty())
    {
      restoreGeometry(m_beforeFullScreenGeometry);
    }
    else
    {
      showMaximized();
    }
    menuBar()->show();
    for (auto* toolbar : findChildren<QToolBar*>())
    {
      toolbar->show();
    }
    statusBar()->show();
  }
}

void MainWindow::showAbout()
{
  QMessageBox::about(this,
    tr("About Traintastic"),
    "<h2>Traintastic v" TRAINTASTIC_VERSION " <small>"
#ifdef TRAINTASTIC_VERSION_EXTRA
      + QString(TRAINTASTIC_VERSION_EXTRA).mid(1) +
#else
      TRAINTASTIC_CODENAME
#endif
      "</small></h2>"
      "<p>" +
      QString(TRAINTASTIC_COPYRIGHT).replace("(c)", "&copy;") +
      "</p>"
      "<p>This program is free software; you can redistribute it and/or"
      " modify it under the terms of the GNU General Public License"
      " as published by the Free Software Foundation; either version 2"
      " of the License, or (at your option) any later version.</p>"
      "<p>This program is distributed in the hope that it will be useful,"
      " but WITHOUT ANY WARRANTY; without even the implied warranty of"
      " MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the"
      " GNU General Public License for more details.</p>"
      "<p><a href=\"https://traintastic.org\">traintastic.org</a></p>");
}
