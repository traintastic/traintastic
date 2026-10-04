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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INPUT_INPUTLOCATION_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INPUT_INPUTLOCATION_HPP

#include <variant>
#include <cstdint>
#include <traintastic/enum/inputchannel.hpp>
#include "../../utils/unreachable.hpp"

struct InputAddress
{
  uint32_t address;
};

struct InputNodeAddress
{
  uint32_t node;
  uint32_t address;
};

using InputLocation = std::variant<InputAddress, InputNodeAddress>;

constexpr bool hasAddressLocation(InputChannel channel) noexcept
{
  switch(channel)
  {
    using enum InputChannel;

    case Input:
    case LocoNet:
    case RBus:
    case S88:
    case S88_Left:
    case S88_Middle:
    case S88_Right:
    case ECoSDetector:
    case ShortEvent:
      return true;

    case LongEvent:
      return false;
  }
  return false; // this should never happen
}

constexpr bool hasNodeAddressLocation(InputChannel channel) noexcept
{
  switch(channel)
  {
    using enum InputChannel;

    case LongEvent:
      return true;

    case Input:
    case LocoNet:
    case RBus:
    case S88:
    case S88_Left:
    case S88_Middle:
    case S88_Right:
    case ECoSDetector:
    case ShortEvent:
      return false;
  }
  return false; // this should never happen
}

constexpr InputLocation inputLocation(InputChannel channel, uint32_t node, uint32_t address) noexcept
{
  if(hasAddressLocation(channel))
  {
    return InputAddress(address);
  }
  if(hasNodeAddressLocation(channel))
  {
    return InputNodeAddress(node, address);
  }
  unreachable();
}

template<>
struct std::hash<InputAddress>
{
  size_t operator()(InputAddress const& value) const noexcept
  {
    return std::hash<decltype(value.address)>{}(value.address);
  }
};

template<>
struct std::hash<InputNodeAddress>
{
  size_t operator()(InputNodeAddress const& value) const noexcept
  {
    return std::hash<decltype(value.node)>{}(value.node) ^ std::hash<decltype(value.address)>{}(value.address);
  }
};

constexpr bool operator ==(const InputAddress& lhs, const InputAddress& rhs) noexcept
{
  return lhs.address == rhs.address;
}

constexpr bool operator ==(const InputNodeAddress lhs, const InputNodeAddress& rhs) noexcept
{
  return lhs.node == rhs.node && lhs.address == rhs.address;
}

constexpr bool operator ==(const InputLocation& lhs, const InputLocation& rhs) noexcept
{
  if(lhs.index() == rhs.index())
  {
    return std::visit(
      [](auto const& l, auto const& r) noexcept -> bool
      {
        return l == r;
      },
      lhs, rhs);
  }
  return false;
}

#endif
