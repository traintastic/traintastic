/**
 * server/src/pcap/pcappipe.cpp
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

#include "pcappipe.hpp"
#ifdef WIN32

#else // unix
  #include <sys/types.h>
  #include <sys/stat.h>
#endif

PCAPPipe::PCAPPipe(std::filesystem::path filename, uint32_t network)
  : m_filename{std::move(filename)}
{
  // try create directory if it doesn't exist
  const auto path = m_filename.parent_path();
  if(!std::filesystem::is_directory(path))
    std::filesystem::create_directories(path);

#ifdef WIN32

#else
  if(std::filesystem::exists(m_filename))
    std::filesystem::remove(m_filename);
  const auto str = m_filename.string();
  if(mkfifo(str.c_str(), 0644) < 0)
    throw std::runtime_error("mkfifo");
  m_stream.open(m_filename, std::ios::binary | std::ios::out);
#endif

  writeHeader(network);
}

PCAPPipe::~PCAPPipe()
{
  m_stream.close();
  std::filesystem::remove(m_filename);
}

void PCAPPipe::write(const void* buffer, size_t size)
{
  m_stream.write(reinterpret_cast<const char*>(buffer), size);
}
