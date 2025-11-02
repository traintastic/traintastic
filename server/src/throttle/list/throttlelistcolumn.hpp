/**
 * server/src/throttle/list/throttlelistcolumn.hpp
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

#ifndef TRAINTASTIC_SERVER_THROTTLE_LIST_THROTTLELISTCOLUMN_HPP
#define TRAINTASTIC_SERVER_THROTTLE_LIST_THROTTLELISTCOLUMN_HPP

#include <type_traits>

enum class ThrottleListColumn
{
  Name = 1 << 0,
  Train = 1 << 1,
  Interface = 1 << 2,
};

constexpr std::array<ThrottleListColumn, 3> throttleListColumnValues = {
  ThrottleListColumn::Name,
  ThrottleListColumn::Train,
  ThrottleListColumn::Interface,
};

constexpr ThrottleListColumn operator|(const ThrottleListColumn lhs, const ThrottleListColumn rhs)
{
  return static_cast<ThrottleListColumn>(static_cast<std::underlying_type_t<ThrottleListColumn>>(lhs) | static_cast<std::underlying_type_t<ThrottleListColumn>>(rhs));
}

constexpr bool contains(const ThrottleListColumn value, const ThrottleListColumn mask)
{
  return (static_cast<std::underlying_type_t<ThrottleListColumn>>(value) & static_cast<std::underlying_type_t<ThrottleListColumn>>(mask)) == static_cast<std::underlying_type_t<ThrottleListColumn>>(mask);
}

#endif
