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
#include <QMessageBox>
#include "simulatorview.hpp"
#include <version.hpp>
#include <traintastic/copyright.hpp>

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
        const auto filename = QFileDialog::getOpenFileName(this, "Load simulation", {}, "*.json");
        if(QFile::exists(filename))
        {
          load(filename);
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
}

void MainWindow::load(const QString& filename)
{
  m_view->setSimulator(new Simulator(filename, m_view));
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
