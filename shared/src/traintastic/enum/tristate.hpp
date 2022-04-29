/**
 * shared/src/enum/tristate.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_TRISTATE_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_TRISTATE_HPP

#include <cstdint>
#include "enum.hpp"

enum class TriState : uint8_t
{
  Undefined = 0,
  False = 1,
  True = 2,
};

TRAINTASTIC_ENUM(TriState, "tri_state", 3,
{
  {TriState::Undefined, "undefined"},
  {TriState::False, "false"},
  {TriState::True, "true"},
});

constexpr TriState toTriState(bool value)
{
  return value ? TriState::True : TriState::False;
}

constexpr TriState operator!(TriState value)
{
  switch(value)
  {
    case TriState::False:
      return TriState::True;
    case TriState::True:
      return TriState::False;
    default:
      return TriState::Undefined;
  }
}

constexpr TriState operator||(TriState lhs, TriState rhs)
{
  if(lhs == TriState::True || rhs == TriState::True)
    return TriState::True;
  else if(lhs == TriState::False && rhs == TriState::False)
    return TriState::False;
  else
    return TriState::Undefined;
}

constexpr TriState operator||(TriState lhs, bool shr)
{
  return lhs || toTriState(shr);
}

constexpr TriState& operator|=(TriState& lhs, TriState shr)
{
  lhs = lhs || shr;
  return lhs;
}

constexpr TriState& operator|=(TriState& lhs, bool shr)
{
  lhs |= toTriState(shr);
  return lhs;
}

constexpr TriState operator&&(TriState lhs, TriState rhs)
{
  if(lhs == TriState::False || rhs == TriState::False)
    return TriState::False;
  else if(lhs == TriState::True && rhs == TriState::True)
    return TriState::True;
  else
    return TriState::Undefined;
}

constexpr TriState operator&&(TriState lhs, bool shr)
{
  return lhs && toTriState(shr);
}

constexpr TriState operator^(TriState lhs, TriState shr)
{
  if(lhs == TriState::Undefined || shr == TriState::Undefined)
    return TriState::Undefined;
  return lhs == shr ? TriState::False : TriState::True;
}

constexpr TriState operator^(TriState lhs, bool shr)
{
  return lhs ^ toTriState(shr);
}

#endif
