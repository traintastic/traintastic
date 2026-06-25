/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#include "ctwmodifier.hpp"
#include "ctwwriter.hpp"

CTWModifier::CTWModifier(const std::filesystem::path& filename)
  : CTWReader(filename)
{
}

CTWModifier::CTWModifier(const std::vector<std::byte>& memory)
  : CTWReader(memory)
{
}

bool CTWModifier::updateFile(const std::filesystem::path& filename, const nlohmann::json& data)
{
  return updateFile(filename, data.dump());
}

bool CTWModifier::updateFile(const std::filesystem::path& filename, const std::string& text)
{
  return updateFile(filename, {reinterpret_cast<const std::byte*>(text.c_str()), text.size()});
}

bool CTWModifier::updateFile(const std::filesystem::path& filename, std::span<const std::byte> bytes)
{
  if(auto it = m_files.find(filename); it != m_files.end())
  {
    it->second.resize(bytes.size());
    std::memcpy(it->second.data(), bytes.data(), it->second.size());
    return true;
  }
  return false;
}

bool CTWModifier::save(const std::filesystem::path& filename)
{
  CTWWriter writer(filename);
  return save(writer);
}

bool CTWModifier::save(std::vector<std::byte>& memory)
{
  CTWWriter writer(memory);
  return save(writer);
}

bool CTWModifier::save(CTWWriter& writer)
{
  for(auto&& [filename, bytes] : m_files)
  {
    writer.writeFile(filename, std::span<const std::byte>(bytes));
  }
  return true;
}
