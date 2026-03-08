/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2024-2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_OUTPUT_OUTPUTTYPES_HPP
#define TRAINTASTIC_SERVER_HARDWARE_OUTPUT_OUTPUTTYPES_HPP

#include <variant>
#include <cstdint>
#include <traintastic/enum/tristate.hpp>
#include <traintastic/enum/outputpairvalue.hpp>

struct OutputAddress
{
  uint32_t address;
};

struct OutputNodeAddress
{
  uint32_t node;
  uint32_t address;
};

struct OutputECoSObject
{
  uint16_t object;
};

using OutputLocation = std::variant<OutputAddress, OutputNodeAddress, OutputECoSObject>;

using OutputValue = std::variant<TriState, OutputPairValue, uint8_t, int16_t>;

template<>
struct std::hash<OutputAddress>
{
  size_t operator()(OutputAddress const& value) const noexcept
  {
    return std::hash<decltype(value.address)>{}(value.address);
  }
};

template<>
struct std::hash<OutputNodeAddress>
{
  size_t operator()(OutputNodeAddress const& value) const noexcept
  {
    return std::hash<decltype(value.node)>{}(value.node) ^ std::hash<decltype(value.address)>{}(value.address);
  }
};

template<>
struct std::hash<OutputECoSObject>
{
  size_t operator()(OutputECoSObject const& value) const noexcept
  {
    return std::hash<decltype(value.object)>{}(value.object);
  }
};

constexpr bool operator ==(const OutputAddress& lhs, const OutputAddress& rhs) noexcept
{
  return lhs.address == rhs.address;
}

constexpr bool operator ==(const OutputNodeAddress lhs, const OutputNodeAddress& rhs) noexcept
{
  return lhs.node == rhs.node && lhs.address == rhs.address;
}

constexpr bool operator ==(const OutputECoSObject& lhs, const OutputECoSObject& rhs) noexcept
{
  return lhs.object == rhs.object;
}

constexpr bool operator ==(const OutputLocation& lhs, const OutputLocation& rhs) noexcept
{
  if(lhs.index() == rhs.index())
  {
    return std::visit(
        [](auto const& l, auto const& r) noexcept -> bool {
            return l == r;
        },
        lhs, rhs);
  }
  return false;
}

#endif
