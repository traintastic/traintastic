/**
 * server/src/os/unix/setgroupid.cpp
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

#include "setgroupid.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <grp.h>
#include <cstring>
#include <stdexcept>

namespace Unix {

void setGroupID(const std::string& groupName)
{
  errno = 0;
  const group* grp = getgrnam(groupName.c_str());

  if(!grp)
  {
    if(errno != 0)
      throw std::runtime_error(std::string("Getting group ").append(groupName).append(" info failed: ").append(std::strerror(errno)));

    throw std::runtime_error("Group " + groupName + " does not exist");
  }

  if(setgid(grp->gr_gid) < 0)
    throw std::runtime_error(std::string("Changing group to ").append(groupName).append(" failed: ").append(std::strerror(errno)));
}

}
