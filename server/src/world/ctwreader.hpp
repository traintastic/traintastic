/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2021-2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_WORLD_CTWREADER_HPP
#define TRAINTASTIC_SERVER_WORLD_CTWREADER_HPP

#include <unordered_map>
#include <vector>
#include <cstddef>
#include <traintastic/utils/stdfilesystem.hpp>
#include <nlohmann/json.hpp>

struct archive;

class CTWReader
{
public:
  CTWReader(const std::filesystem::path& filename);
  CTWReader(const std::vector<std::byte>& memory);

  bool readFile(const std::filesystem::path& filename, nlohmann::json& data);
  bool readFile(const std::filesystem::path& filename, std::string& text);

protected:
  std::unordered_map<std::string, std::vector<std::byte>> m_files;

private:
  std::unique_ptr<archive, void(*)(archive*)> m_archive;

  CTWReader();
  void readFiles();
};

#endif
