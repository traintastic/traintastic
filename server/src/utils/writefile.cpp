/**
 * server/src/utils/writefile.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#include "writefile.hpp"
#include <fstream>

//! \brief Try to create directories if they doesn't exist already
static bool createDirectories(const std::filesystem::path& path)
{
  if(!std::filesystem::is_directory(path))
  {
    std::error_code ec;
    std::filesystem::create_directories(path, ec);
    if(ec)
      return false;
  }
  return true;
}

bool writeFile(const std::filesystem::path& filename, const void* data, size_t dataSize)
{
  if(!createDirectories(filename.parent_path()))
    return false;

  std::ofstream file;
  file.open(filename, std::ios::binary | std::ios::out | std::ios::trunc);
  if(!file.is_open())
    return false;

  file.write(reinterpret_cast<const char*>(data), dataSize);

  return true;
}

bool writeFileJSON(const std::filesystem::path& filename, const nlohmann::json& data)
{
  if(!createDirectories(filename.parent_path()))
    return false;

  std::ofstream file;
  file.open(filename, std::ios::binary | std::ios::out | std::ios::trunc);
  if(!file.is_open())
    return false;

  file << data.dump(2);

  return true;
}
