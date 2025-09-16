/**
 * server/src/utils/endswith.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022,2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_UTILS_ENDSWITH_HPP
#define TRAINTASTIC_SERVER_UTILS_ENDSWITH_HPP

#include <string_view>

constexpr bool endsWith(std::string_view sv, std::string_view suffix)
{
  return sv.size() >= suffix.size() && std::equal(suffix.rbegin(), suffix.rend(), sv.rbegin());
}

constexpr bool endsWith(std::string_view sv, std::span<const std::string_view> suffixes)
{
  for(auto suffix : suffixes)
  {
    if(endsWith(sv, suffix))
    {
      return true;
    }
  }
  return false;
}

#endif
