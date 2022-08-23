/**
 * server/src/os/unix/writepidfile.cpp
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

#include "writepidfile.hpp"
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <stdexcept>

namespace Unix {

void writePIDFile(const std::filesystem::path& filename)
{
  std::ofstream pidFile(filename, std::ofstream::out | std::ofstream::trunc);
  if(!pidFile.is_open())
    throw std::runtime_error(std::string("Opening PID file ").append(filename.string()).append("failed"));
  pidFile << getpid() << std::endl;
}

}
