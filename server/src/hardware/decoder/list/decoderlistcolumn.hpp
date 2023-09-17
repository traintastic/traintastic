/**
 * server/src/hardware/decoder/list/decoderlistcolumn.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_DECODER_LIST_DECODERLISTCOLUMN_HPP
#define TRAINTASTIC_SERVER_HARDWARE_DECODER_LIST_DECODERLISTCOLUMN_HPP

#include <type_traits>
#include <array>

enum class DecoderListColumn
{
  Id = 1 << 0,
  Name = 1 << 1,
  Interface = 1 << 2,
  Address = 1 << 3,
  Protocol = 1 << 4,
};

constexpr std::array<DecoderListColumn, 5> decoderListColumnValues = {
  DecoderListColumn::Id,
  DecoderListColumn::Name,
  DecoderListColumn::Interface,
  DecoderListColumn::Protocol,
  DecoderListColumn::Address,
};

constexpr DecoderListColumn operator|(const DecoderListColumn lhs, const DecoderListColumn rhs)
{
  return static_cast<DecoderListColumn>(static_cast<std::underlying_type_t<DecoderListColumn>>(lhs) | static_cast<std::underlying_type_t<DecoderListColumn>>(rhs));
}

constexpr bool contains(const DecoderListColumn value, const DecoderListColumn mask)
{
  return (static_cast<std::underlying_type_t<DecoderListColumn>>(value) & static_cast<std::underlying_type_t<DecoderListColumn>>(mask)) == static_cast<std::underlying_type_t<DecoderListColumn>>(mask);
}

#endif
