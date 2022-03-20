/**
 * shared/src/traintastic/enum/directioncontrolstate.hpp
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_DIRECTIONCONTROLSTATE_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_DIRECTIONCONTROLSTATE_HPP

#include <cstdint>
#include "enum.hpp"

enum class DirectionControlState : uint8_t
{
  None = 0,
  AtoB = 1,
  BtoA = 2,
  Both = 3,
};

template<>
struct EnumName<DirectionControlState>
{
  static constexpr char const* value = "direction_control_state";
};

ENUM_VALUES(DirectionControlState, 4,
{
  {DirectionControlState::None, "none"},
  {DirectionControlState::AtoB, "a_to_b"},
  {DirectionControlState::BtoA, "b_to_a"},
  {DirectionControlState::Both, "both"},
})

#endif
