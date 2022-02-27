/**
 * server/src/world/ctwwriter.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_WORLD_CTWWRITER_HPP
#define TRAINTASTIC_SERVER_WORLD_CTWWRITER_HPP

#include <memory>
#include <vector>
#include <traintastic/utils/stdfilesystem.hpp>
#include <nlohmann/json.hpp>

struct archive;

class CTWWriter
{
  private:
    std::unique_ptr<archive, void(*)(archive*)> m_archive;

    CTWWriter();

  public:
    CTWWriter(const std::filesystem::path& filename);
    CTWWriter(std::vector<std::byte>& memory);

    void writeFile(const std::filesystem::path& filename, const nlohmann::json& data);
    void writeFile(const std::filesystem::path& filename, const std::string& text);
};

#endif
