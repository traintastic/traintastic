/**
 * server/src/hardware/output/list/outputlistcolumn.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022,2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_OUTPUT_LIST_OUTPUTLISTCOLUMN_HPP
#define TRAINTASTIC_SERVER_HARDWARE_OUTPUT_LIST_OUTPUTLISTCOLUMN_HPP

#include <type_traits>

enum class OutputListColumn
{
  Interface = 1 << 0,
  Channel = 1 << 1,
  Address = 1 << 2,
};

constexpr std::array<OutputListColumn, 3> outputListColumnValues = {
  OutputListColumn::Interface,
  OutputListColumn::Channel,
  OutputListColumn::Address,
};

constexpr OutputListColumn operator|(const OutputListColumn lhs, const OutputListColumn rhs)
{
  return static_cast<OutputListColumn>(static_cast<std::underlying_type_t<OutputListColumn>>(lhs) | static_cast<std::underlying_type_t<OutputListColumn>>(rhs));
}

constexpr bool contains(const OutputListColumn value, const OutputListColumn mask)
{
  return (static_cast<std::underlying_type_t<OutputListColumn>>(value) & static_cast<std::underlying_type_t<OutputListColumn>>(mask)) == static_cast<std::underlying_type_t<OutputListColumn>>(mask);
}

#endif
