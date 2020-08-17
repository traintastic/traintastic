/**
 * client/src/main.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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
#ifdef Q_OS_WINDOWS
  #include <QSettings>
  //#include <QStandardPaths>
#endif
#include "mainwindow.hpp"
#include "utils/getlocalepath.hpp"
//#include "network/client.hpp"



#include <traintastic/locale/locale.hpp>


int main(int argc, char* argv[])
{
  QApplication::setOrganizationName("Traintastic");
  QApplication::setOrganizationDomain("traintastic.org");
  QApplication::setApplicationName("traintastic-client");
  QApplication::setApplicationVersion("0.0.1");

#ifdef Q_OS_WINDOWS
  QSettings::setDefaultFormat(QSettings::IniFormat);
#endif

  QApplication app(argc, argv);

  //Client client;
  //Client::instance = &client;


  Locale::instance = new Locale(getLocalePath().toStdString() + "/en-us.txt");

  MainWindow mw;
  mw.show();

  if(!mw.connection())
    mw.connectToServer();

  return app.exec();
}
