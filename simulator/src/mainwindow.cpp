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
#include "simulatorview.hpp"
#include <version.hpp>
#include <traintastic/copyright.hpp>
#include <traintastic/utils/standardpaths.hpp>

#include <QMessageBox>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags)
  : QMainWindow(parent, flags)
  , m_view{new SimulatorView(this)}
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
          QMetaObject::invokeMethod(m_view, &SimulatorView::zoomToFit, Qt::QueuedConnection);
        }
      });
    menu->addAction("Quit", this, &MainWindow::close);

    menu = menuBar()->addMenu("View");
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

  connect(m_view, &SimulatorView::powerOnChanged, m_power, &QAction::setChecked);
}

void MainWindow::load(const QString& filename)
{
  setWindowFilePath(filename);

  QFile file(filename);
  if(file.open(QIODeviceBase::ReadOnly))
  {
    try
    {
      m_view->setSimulator(std::make_shared<Simulator>(nlohmann::json::parse(file.readAll().toStdString())));
    }
    catch(std::exception &e)
    {
      qDebug() << "Error loading:" << filename << e.what();
      QMessageBox::warning(this, tr("Load Error"),
                           e.what());
      return;
    }

  }
}

void MainWindow::keyPressEvent(QKeyEvent *ev)
{
  if(ev->modifiers() == Qt::ControlModifier && ev->key() == Qt::Key_L)
  {
    // Reload if not power on
    if(windowFilePath().isEmpty() || m_power->isChecked())
      return;

    const float zoomLevel = m_view->getZoomLevel();
    const auto &cameraPt = m_view->getCamera();

    load(windowFilePath());

    m_view->setZoomLevel(zoomLevel);
    m_view->setCamera(cameraPt);

    return;
  }
  QMainWindow::keyPressEvent(ev);
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
