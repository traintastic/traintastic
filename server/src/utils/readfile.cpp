/**
 * server/src/utils/readfile.cpp
 *
 * This file is part of the traintastic source code.
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

#include "readfile.hpp"
#include <fstream>

std::optional<std::string> readFile(const std::filesystem::path& filename)
{
  std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
  if(!file.is_open())
  {
    return std::nullopt;
  }
  const size_t size = file.tellg();
  std::string contents;
  contents.resize(size);
  file.seekg(std::ios::beg);
  file.read(contents.data(), size);
  return contents;
}
