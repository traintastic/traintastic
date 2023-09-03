/**
 * server/src/utils/writefile.hpp
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

#ifndef TRAINTASTIC_SERVER_UTILS_WRITEFILE_HPP
#define TRAINTASTIC_SERVER_UTILS_WRITEFILE_HPP

#include <cstddef>
#include <vector>
#include <filesystem>
#include <nlohmann/json.hpp>

bool writeFile(const std::filesystem::path& filename, const void* data, size_t dataSize);

inline bool writeFile(const std::filesystem::path& filename, std::string_view data)
{
  return writeFile(filename, data.data(), data.size());
}

template<typename T>
inline bool writeFile(const std::filesystem::path& filename, const std::vector<T>& data)
{
  return writeFile(filename, data.data(), data.size() * sizeof(T));
}

bool writeFileJSON(const std::filesystem::path& filename, const nlohmann::json& data);

#endif
