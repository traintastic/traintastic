/**
 * client/src/main.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021,2023 Reinder Feenstra
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
#endif
#include <QCommandLineParser>
#include <version.hpp>
#include "mainwindow.hpp"
#include "settings/generalsettings.hpp"
#include "settings/developersettings.hpp"
#include "style/materialdarkstyle.hpp"
#include "style/materiallightstyle.hpp"
#include "theme/theme.hpp"
#include <traintastic/locale/locale.hpp>
#include <traintastic/utils/standardpaths.hpp>

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

static void saveMissing(QDir path, const Locale& locale)
{
  QFile file(path.path() + QDir::separator() + "missing." + QString::fromStdString(locale.filename.filename().string()));

  if(file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text) && locale.missing())
  {
    for(const auto& id : *locale.missing())
    {
      file.write(id.data(), id.size());
      file.write("=\n");
    }
  }
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
  const QString languageDefault = GeneralSettings::instance().language.defaultValue();
  const QString language = GeneralSettings::instance().language;

  const bool logMissingStrings = !DeveloperSettings::instance().logMissingStringsDir.value().isEmpty();

  const auto localePath = getLocalePath();
  Locale::instance = std::make_unique<Locale>(localePath / "neutral.lang");
  if(language != languageDefault && !DeveloperSettings::instance().dontLoadFallbackLanguage)
  {
    Locale::instance = std::make_unique<Locale>(localePath / languageDefault.toStdString().append(".lang"), std::move(Locale::instance));
    if(logMissingStrings)
      const_cast<Locale*>(Locale::instance.get())->enableMissingLogging();
  }

  Locale::instance = std::make_unique<Locale>(localePath / language.toStdString().append(".lang"), std::move(Locale::instance));
  if(logMissingStrings)
    const_cast<Locale*>(Locale::instance.get())->enableMissingLogging();

  // Auto select icon set based on background color lightness:
  const qreal backgroundLightness = QApplication::style()->standardPalette().window().color().lightnessF();
  Theme::setIconSet(backgroundLightness < 0.5 ? Theme::IconSet::Dark : Theme::IconSet::Light);

  MainWindow mw;
  if(options.fullscreen)
    mw.showFullScreen();
  else
    mw.show();

  if(!mw.connection())
    mw.connectToServer(options.connectTo);

  const unsigned  int r = app.exec();

  if(logMissingStrings)
  {
    auto dir = DeveloperSettings::instance().logMissingStringsDir.value();
    if(QDir(dir).exists())
    {
      if(Locale::instance->fallback())
        saveMissing(dir, *Locale::instance->fallback());
      saveMissing(dir, *Locale::instance);
    }
  }

  return r;
}
