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

#include <QApplication>
#include <version.hpp>
#include "mainwindow.hpp"

int main(int argc, char* argv[])
{
  QApplication::setOrganizationName("Traintastic");
  QApplication::setOrganizationDomain("traintastic.org");
  QApplication::setApplicationName("traintastic-simulator");
  QApplication::setApplicationVersion(TRAINTASTIC_VERSION);

  QApplication app(argc, argv);

  MainWindow mw;
  if(argc >= 2)
  {
    mw.load(argv[1]);
  }
  mw.show();

  return app.exec();
}
