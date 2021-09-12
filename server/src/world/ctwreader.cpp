/**
 * server/src/world/ctwreader.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#include "ctwreader.hpp"
#include <archive.h>
#include <archive_entry.h>
#include "libarchiveerror.hpp"

using nlohmann::json;

CTWReader::CTWReader(const std::filesystem::path& filename)
{
  std::unique_ptr<archive, void(*)(archive*)> ctw{archive_read_new(),
    [](archive* a)
    {
      archive_read_close(a);
      archive_read_free(a);
    }};

  if(archive_read_support_filter_xz(ctw.get()) != ARCHIVE_OK)
    throw LibArchiveError(ctw.get());
  if(archive_read_support_format_tar(ctw.get()) != ARCHIVE_OK)
    throw LibArchiveError(ctw.get());
  if(archive_read_open_filename(ctw.get(), filename.string().c_str(), 10240) != ARCHIVE_OK)
    throw LibArchiveError(ctw.get());

  // load everything in memory:
  archive_entry* entry = nullptr;
  while(true)
  {
    const int r = archive_read_next_header(ctw.get(), &entry);
    if(r == ARCHIVE_EOF)
      break;
    else if(r < ARCHIVE_OK)
      throw LibArchiveError(ctw.get());

    std::vector<std::byte> data;
    data.resize(archive_entry_size(entry));

    size_t pos = 0;
    while(pos < data.size())
    {
      const auto count = archive_read_data(ctw.get(), data.data() + pos, data.size() - pos);
      if(count < 0)
        throw LibArchiveError(ctw.get());
      else if(count == 0)
        break; // should not happen
      pos += static_cast<size_t>(count);
    }

    if(pos == data.size())
      m_files.emplace(archive_entry_pathname(entry), std::move(data));
  }
}

bool CTWReader::readFile(const std::filesystem::path& filename, nlohmann::json& data)
{
  auto it = m_files.find(filename.string());
  if(it == m_files.end())
    return false;

  std::string_view sv{reinterpret_cast<const char*>(it->second.data()), it->second.size()};
  data = json::parse(sv.begin(), sv.end());
  return true;
}

bool CTWReader::readFile(const std::filesystem::path& filename, std::string& text)
{
  auto it = m_files.find(filename.string());
  if(it == m_files.end())
    return false;

  text.assign(reinterpret_cast<const char*>(it->second.data()), it->second.size());
  return true;
}
