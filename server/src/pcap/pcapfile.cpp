/**
 * server/src/pcap/pcapfile.cpp
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

#include "pcapfile.hpp"

PCAPFile::PCAPFile(const std::filesystem::path& filename, uint32_t network)
{
  // try create directory if it doesn't exist
  const auto path = filename.parent_path();
  if(!std::filesystem::is_directory(path))
    std::filesystem::create_directories(path);

  m_stream.open(filename, std::ios::binary | std::ios::out | std::ios::trunc);

  writeHeader(network);
}

PCAPFile::~PCAPFile()
{
  m_stream.close();
}

void PCAPFile::write(const void* buffer, size_t size)
{
  m_stream.write(reinterpret_cast<const char*>(buffer), size);
  m_stream.flush();
}
