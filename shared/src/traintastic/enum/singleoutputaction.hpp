/**
 * shared/src/traintastic/enum/singleoutputaction.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022,2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_SINGLEOUTPUTACTION_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_SINGLEOUTPUTACTION_HPP

#include <cstdint>
#include <array>
#include "enum.hpp"

enum class SingleOutputAction : uint8_t
{
  None = 0,
  Off = 1,
  On = 2,
  Pulse = 3,
};

TRAINTASTIC_ENUM(SingleOutputAction, "single_output_action", 4,
{
  {SingleOutputAction::None, "none"},
  {SingleOutputAction::Off, "off"},
  {SingleOutputAction::On, "on"},
  {SingleOutputAction::Pulse, "pulse"},
});

inline constexpr std::array<SingleOutputAction, 4> singleOutputActionValues{{
  SingleOutputAction::None,
  SingleOutputAction::Off,
  SingleOutputAction::On,
  SingleOutputAction::Pulse,
}};

#endif
