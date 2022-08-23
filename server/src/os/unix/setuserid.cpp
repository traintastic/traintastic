/**
 * server/src/os/unix/setuserid.cpp
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

#include "setuserid.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <grp.h>
#include <pwd.h>
#include <cstring>
#include <stdexcept>

namespace Unix {

void setUserID(const std::string& username, bool setUserDefaultGroup)
{
  errno = 0;
  const passwd* user = getpwnam(username.c_str());
  if(!user)
  {
    if(errno != 0)
      throw std::runtime_error(std::string("Getting user ").append(username).append(" info failed: ").append(std::strerror(errno)));

    throw std::runtime_error("User " + username + " does not exist");
  }

  const gid_t gid = setUserDefaultGroup ? user->pw_gid : getgid();

  if(initgroups(username.c_str(), gid) < 0)
    throw std::runtime_error(std::string("Initializing the supplementary groups for user ").append(username).append(" failed: ").append(std::strerror(errno)));

  if(setUserDefaultGroup && setgid(gid) < 0)
    throw std::runtime_error(std::string("Changing group id to ").append(std::to_string(gid)).append(" failed: ").append(std::strerror(errno)));

  if(setuid(user->pw_uid) < 0)
    throw std::runtime_error(std::string("Changing user to ").append(username).append(" failed: ").append(std::strerror(errno)));
}

}
