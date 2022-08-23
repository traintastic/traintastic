/**
 * server/src/os/unix/daemonize.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#include "daemonize.hpp"
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>

namespace Unix {

void daemonize(const std::filesystem::path& workingDirectory)
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

  // create a new SID for the child process:
  if(setsid() < 0)
  {
    std::cerr << "Failed to create new session" << std::endl;
    exit(EXIT_FAILURE);
  }

  // change work directory to root:
  const auto wd = workingDirectory.string();
  if(chdir(wd.c_str()) < 0)
  {
    std::cerr << "Failed to change work directory to " << wd << std::endl;
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

}

