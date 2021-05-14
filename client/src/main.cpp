/**
 * client/src/main.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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
#include <QCommandLineParser>
#include <QSettings>
#include <version.hpp>
#include "mainwindow.hpp"
#include "utils/getlocalepath.hpp"
//#include "network/client.hpp"


#include "style/materialdarkstyle.hpp"
#include "style/materiallightstyle.hpp"

#include <traintastic/locale/locale.hpp>

struct Options
{
  bool fullscreen;
  QString connectTo;
};

void parseOptions(QCoreApplication& app, Options& options)
{
  QCommandLineParser parser;
  parser.setApplicationDescription("Traintastic client");
  parser.addHelpOption();
  parser.addVersionOption();

  QCommandLineOption fullscreen("fullscreen", "Start application fullscreen.");
  parser.addOption(fullscreen);

  QCommandLineOption connect({"c", "connect"}, "Connect to server.", "hostname[:port]");
  parser.addOption(connect);

  parser.process(app);

  options.fullscreen = parser.isSet(fullscreen);
  options.connectTo = parser.value(connect);
}

int main(int argc, char* argv[])
{
  QApplication::setOrganizationName("Traintastic");
  QApplication::setOrganizationDomain("traintastic.org");
  QApplication::setApplicationName("traintastic-client");
  QApplication::setApplicationVersion(TRAINTASTIC_VERSION);

  //QApplication::setStyle(new MaterialDarkStyle());
  //QApplication::setStyle(new MaterialLightStyle());

#ifdef Q_OS_WINDOWS
  QSettings::setDefaultFormat(QSettings::IniFormat);
#endif

  QApplication app(argc, argv);

  Options options;
  parseOptions(app, options);

  // language:
  const QString languageDefault = "en-us";
  QString language = QSettings().value("language", languageDefault).toString();

  Locale* fallback = nullptr;
  if(language != languageDefault)
    fallback = new Locale(getLocalePath().toStdString() + "/" + languageDefault.toStdString() + ".txt");

  Locale::instance = new Locale(getLocalePath().toStdString() + "/" + language.toStdString() + ".txt", fallback);

  MainWindow mw;
  if(options.fullscreen)
    mw.showFullScreen();
  else
    mw.show();

  if(!mw.connection())
    mw.connectToServer(options.connectTo);

  return app.exec();
}
