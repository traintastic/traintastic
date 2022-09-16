/**
 * server/src/hardware/identification/list/identificationlistcolumn.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_IDENTIFICATION_LIST_IDENTIFICATIONLISTCOLUMN_HPP
#define TRAINTASTIC_SERVER_HARDWARE_IDENTIFICATION_LIST_IDENTIFICATIONLISTCOLUMN_HPP

#include <type_traits>

enum class IdentificationListColumn
{
  Id = 1 << 0,
  Name = 1 << 1,
  Interface = 1 << 2,
  Channel = 1 << 3,
  Address = 1 << 4,
};

constexpr std::array<IdentificationListColumn, 5> identificationListColumnValues = {
  IdentificationListColumn::Id,
  IdentificationListColumn::Name,
  IdentificationListColumn::Interface,
  IdentificationListColumn::Channel,
  IdentificationListColumn::Address,
};

constexpr IdentificationListColumn operator|(const IdentificationListColumn lhs, const IdentificationListColumn rhs)
{
  return static_cast<IdentificationListColumn>(static_cast<std::underlying_type_t<IdentificationListColumn>>(lhs) | static_cast<std::underlying_type_t<IdentificationListColumn>>(rhs));
}

constexpr bool contains(const IdentificationListColumn value, const IdentificationListColumn mask)
{
  return (static_cast<std::underlying_type_t<IdentificationListColumn>>(value) & static_cast<std::underlying_type_t<IdentificationListColumn>>(mask)) == static_cast<std::underlying_type_t<IdentificationListColumn>>(mask);
}

#endif
