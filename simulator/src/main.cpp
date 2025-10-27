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
#include <QCommandLineParser>
#ifdef Q_OS_WINDOWS
  #include <QSettings>
#endif
#include <version.hpp>
#include "mainwindow.hpp"

struct Options
{
  bool fullscreen = false;
  bool localOnly = false;
  bool discoverable = false;
  QString filename;
};

void parseOptions(QCoreApplication& app, Options& options)
{
  QCommandLineParser parser;
  parser.setApplicationDescription("Traintastic simulator");
  parser.addHelpOption();
  parser.addVersionOption();

  QCommandLineOption fullscreen("fullscreen", "Start application fullscreen.");
  parser.addOption(fullscreen);

  QCommandLineOption localOnly("local", "Accept localhost only.");
  parser.addOption(localOnly);

  QCommandLineOption discoverable("discoverable", "Enable UDP discovery");
  parser.addOption(discoverable);

  parser.addPositionalArgument("filename", "File to load.");

  parser.process(app);

  options.fullscreen = parser.isSet(fullscreen);
  options.localOnly = !parser.isSet(localOnly);
  options.discoverable = parser.isSet(discoverable);

  if(!parser.positionalArguments().isEmpty())
  {
    options.filename = parser.positionalArguments().first();
  }
}

int main(int argc, char* argv[])
{
  QApplication::setOrganizationName("Traintastic");
  QApplication::setOrganizationDomain("traintastic.org");
  QApplication::setApplicationName("traintastic-simulator");
  QApplication::setApplicationVersion(TRAINTASTIC_VERSION);

#ifdef Q_OS_WINDOWS
  QSettings::setDefaultFormat(QSettings::IniFormat);
#endif

  QApplication app(argc, argv);

  Options options;
  parseOptions(app, options);

  MainWindow mw;
  mw.setOptions(options.localOnly, options.discoverable);

  mw.show();
  if(!options.filename.isEmpty())
  {
    mw.load(options.filename);
  }
  if(options.fullscreen)
  {
    mw.setFullScreen(true);
  }

  return app.exec();
}
