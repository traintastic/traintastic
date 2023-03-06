/**
 * shared/src/traintastic/enum/turnoutposition.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_TURNOUTPOSITION_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_TURNOUTPOSITION_HPP

#include <cstdint>
#include "enum.hpp"

enum class TurnoutPosition : uint8_t
{
  Unknown = 0,
  Straight = 1,
  Left = 2,
  Right = 3,
  Crossed = 4,
  Diverged = 5,
  DoubleSlipStraightA = 6,
  DoubleSlipStraightB = 7
};

TRAINTASTIC_ENUM(TurnoutPosition, "turnout_position", 8,
{
  {TurnoutPosition::Unknown, "unknown"},
  {TurnoutPosition::Straight, "straight"},
  {TurnoutPosition::Left, "left"},
  {TurnoutPosition::Right, "right"},
  {TurnoutPosition::Crossed, "crossed"},
  {TurnoutPosition::Diverged, "diverged"},
  {TurnoutPosition::DoubleSlipStraightA, "double_slip_straight_a"},
  {TurnoutPosition::DoubleSlipStraightB, "double_slip_straight_b"}
});

#endif
