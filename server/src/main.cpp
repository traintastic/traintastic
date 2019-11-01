/**
 * Traintastic
 *
 * Copyright (C) 2019 Reinder Feenstra <reinderfeenstra@gmail.com>
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
#include "core/traintastic.hpp"








#include "lua/sandbox.hpp"
lua_State* L = Lua::newSandbox();





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
          Traintastic::instance->console->notice(Traintastic::id, std::string("Received signal: ") + strsignal(signum));
          Traintastic::instance->shutdown();
        });
      break;
    }
    default:
      EventLoop::call(
        [signum]()
        {
          Traintastic::instance->console->warning(Traintastic::id, std::string("Received unknown signal: ") + strsignal(signum));
        });
      break;
  }
}
#endif

int main(int argc, char* argv[])
{
  int status = EXIT_SUCCESS;

  // parse command line options:
  const Options options(argc, argv);

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
#endif

  EventLoop::start();

  try
  {
    Traintastic::instance = std::make_unique<Traintastic>(options.dataDir);
    status = Traintastic::instance->run() ? EXIT_SUCCESS : EXIT_FAILURE;
    Traintastic::instance.reset();
  }
  catch(const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
    status = EXIT_FAILURE;
  }

  EventLoop::stop();

  return status;
}
