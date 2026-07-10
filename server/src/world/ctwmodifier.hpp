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

#ifndef TRAINTASTIC_SERVER_WORLD_CTWMODIFIER_HPP
#define TRAINTASTIC_SERVER_WORLD_CTWMODIFIER_HPP

#include "ctwreader.hpp"
#include <span>

class CTWWriter;

class CTWModifier : public CTWReader
{
public:
  CTWModifier(const std::filesystem::path& filename);
  CTWModifier(const std::vector<std::byte>& memory);

  bool updateFile(const std::filesystem::path& filename, const nlohmann::json& data);
  bool updateFile(const std::filesystem::path& filename, const std::string& text);
  bool updateFile(const std::filesystem::path& filename, std::span<const std::byte> bytes);

  bool save(const std::filesystem::path& filename);
  bool save(std::vector<std::byte>& memory);

private:
  bool save(CTWWriter& writer);
};

#endif
