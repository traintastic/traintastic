/**
 * shared/src/traintastic/enum/pairoutputaction.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_PAIROUTPUTACTION_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_PAIROUTPUTACTION_HPP

#include <cstdint>
#include <array>
#include "enum.hpp"

enum class PairOutputAction : uint8_t
{
  None = 0,
  First = 1,
  Second = 2,
};

TRAINTASTIC_ENUM(PairOutputAction, "pair_output_action", 3,
{
  {PairOutputAction::None, "none"},
  {PairOutputAction::First, "first"},
  {PairOutputAction::Second, "second"},
});

inline constexpr std::array<PairOutputAction, 3> pairOutputActionValues{{
  PairOutputAction::None,
  PairOutputAction::First,
  PairOutputAction::Second,
}};

#endif
