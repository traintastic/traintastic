/**
 * server/src/main.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022 Reinder Feenstra
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

#include <iostream>
#include "options.hpp"
#include "traintastic/traintastic.hpp"
#include "log/log.hpp"
#include <traintastic/locale/locale.hpp>
#include <traintastic/utils/standardpaths.hpp>
#ifdef __unix__
  #include "os/unix/daemonize.hpp"
  #include "os/unix/writepidfile.hpp"
  #include "os/unix/setgroupid.hpp"
  #include "os/unix/setuserid.hpp"
#elif defined(WIN32)
  #include "os/windows/consolewindow.hpp"
  #include "os/windows/trayicon.hpp"
#endif

int main(int argc, char* argv[])
{
  // parse command line options:
  const Options options(argc, argv);

  std::filesystem::path dataDir{options.dataDir};
  if(dataDir.empty())
  {
#ifdef __unix__
    if(const char* home = getenv("HOME"))
      dataDir += std::filesystem::path(home) / ".config" / "traintastic-server";
#elif defined(WIN32)
    dataDir = getLocalAppDataPath() / "traintastic" / "server";
#endif
  }

  bool enableConsoleLogger = true;

#ifdef __unix__
  if(options.daemonize)
  {
    enableConsoleLogger = false;
    Unix::daemonize(dataDir);
  }

  try
  {
    if(!options.pidFile.empty())
      Unix::writePIDFile(options.pidFile);

    if(!options.group.empty())
      Unix::setGroupID(options.group);

    if(!options.user.empty())
      Unix::setUserID(options.user, options.group.empty());
  }
  catch(const std::exception& e)
  {
    if(!options.daemonize)
      std::cerr << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }
#elif defined(WIN32)
  if(options.tray)
  {
    Windows::setConsoleWindowVisible(false);
    enableConsoleLogger = Windows::hasConsoleWindow();
  }
#endif

  Locale::instance = new Locale(getLocalePath() / "en-us.txt");

  if(enableConsoleLogger)
    Log::enableConsoleLogger();

  int status = EXIT_SUCCESS;
  bool restart = false;

  do
  {
    {
      const auto settings = Settings::getPreStartSettings(dataDir);

      if(settings.memoryLoggerSize > 0)
        Log::enableMemoryLogger(settings.memoryLoggerSize);
      else
        Log::disableMemoryLogger();

      if(settings.enableFileLogger)
        Log::enableFileLogger(dataDir / "log" / "traintastic.txt");
      else
        Log::disableFileLogger();
    }

#ifdef WIN32
    if(options.tray)
      Windows::TrayIcon::add(restart);
#endif

    try
    {
      Traintastic::instance = std::make_shared<Traintastic>(dataDir);

      if(!restart) // initial startup
        status = Traintastic::instance->run(options.world, options.simulate, options.online, options.power, options.run);
      else
        status = Traintastic::instance->run();

      switch(status)
      {
        case Traintastic::ExitSuccess:
          restart = false;
          status = EXIT_SUCCESS;
          break;

        case Traintastic::Restart:
          restart = true;
          break;

        case Traintastic::ExitFailure:
          restart = false;
          status = EXIT_FAILURE;
          break;
      }
    }
    catch(const std::exception& e)
    {
      std::cerr << e.what() << std::endl;
      restart = false;
      status = EXIT_FAILURE;
    }

#ifdef WIN32
    if(options.tray)
      Windows::TrayIcon::remove();
#endif

    if(Traintastic::instance)
    {
#ifndef NDEBUG
      std::weak_ptr<Traintastic> weak = Traintastic::instance;
#endif
      Traintastic::instance->destroy(); // notify others to release the object
      Traintastic::instance.reset();
#ifndef NDEBUG
      assert(weak.expired());
#endif
    }
  }
  while(restart);

  return status;
}
