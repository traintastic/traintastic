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
#ifdef __unix__
  #include <csignal>
#endif
#include "options.hpp"
#include "core/eventloop.hpp"
#include "traintastic/traintastic.hpp"
#include "log/log.hpp"
#include <traintastic/locale/locale.hpp>
#include <traintastic/utils/standardpaths.hpp>
#ifdef WIN32
  #include "os/windows/consolewindow.hpp"
  #include "os/windows/trayicon.hpp"
#endif

#ifdef __unix__
void signalHandler(int signum)
{
  switch(signum)
  {
    case SIGINT:
    case SIGQUIT:
    {
      signal(SIGINT, SIG_DFL);
      signal(SIGQUIT, SIG_DFL);

      EventLoop::call(
        [signum]()
        {
          Log::log(*Traintastic::instance, LogMessage::N1001_RECEIVED_SIGNAL_X, std::string_view{strsignal(signum)});
          Traintastic::instance->exit();
        });
      break;
    }
  }
}
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

  Locale::instance = new Locale(getLocalePath() / "en-us.txt");

  bool enableConsoleLogger = true;

#ifdef __unix__
/*
  if(options.daemonize)
  {
    // fork first time:
    pid_t pid = fork();
    if(pid < 0)
    {
      std::cerr << "First time fork failed" << std::endl;
      exit(EXIT_FAILURE);
    }
    else if(pid > 0)
      exit(EXIT_SUCCESS); // exit parent process

    // Create a new SID for the child process:
    if(setsid() < 0)
    {
      std::cerr << "Failed to create new session" << std::endl;
      exit(EXIT_FAILURE);
    }

    // change work directory to root:
    if(chdir("/") < 0)
    {
      std::cerr << "Failed to change work directory to root" << std::endl;
      exit(EXIT_FAILURE);
    }

    // change the file mode mask:
    umask(0);

    // fork second time:
    pid = fork();
    if(pid < 0)
    {
      std::cerr << "Second time fork failed" << std::endl;
      exit(EXIT_FAILURE);
    }
    else if(pid > 0)
      exit(EXIT_SUCCESS); // Exit parent process.

    // close all std io:
    fclose(stdin);
    fclose(stdout);
    fclose(stderr);
  }

  if(!options.pidFile.empty())
    UnixUtils::writePIDFile(options.pidFile);

  if(!options.group.empty())
  {
    UnixUtils::changeGroupID(options.group);
  }

  if(!options.user.empty())
  {
    UnixUtils::changeUserID(options.user, options.group.empty(), true);
  }
*/

  // setup signal handlers:
  signal(SIGINT, signalHandler);
  signal(SIGQUIT, signalHandler);
#elif defined(WIN32)
  if(options.tray)
  {
    Windows::setConsoleWindowVisible(false);
    enableConsoleLogger = false;
  }
#endif

  if(enableConsoleLogger)
    Log::enableConsoleLogger();

  int status = EXIT_SUCCESS;
  bool restart = false;

  do
  {
    restart = false;

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

    EventLoop::start();
#ifdef WIN32
    if(options.tray)
      Windows::TrayIcon::add();
#endif

    try
    {
      Traintastic::instance = std::make_shared<Traintastic>(dataDir);

      switch(Traintastic::instance->run())
      {
        case Traintastic::ExitSuccess:
          status = EXIT_SUCCESS;
          break;

        case Traintastic::Restart:
          restart = true;
          break;

        case Traintastic::ExitFailure:
          status = EXIT_FAILURE;
          break;
      }
    }
    catch(const std::exception& e)
    {
      std::cerr << e.what() << std::endl;
      status = EXIT_FAILURE;
    }

#ifdef WIN32
    if(options.tray)
      Windows::TrayIcon::remove();
#endif
    EventLoop::stop();
  }
  while(restart);

  return status;
}
