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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_INPUTCONDITION_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_INPUTCONDITION_HPP

#include <cstdint>
#include <array>
#include "enum.hpp"

enum class InputCondition : uint8_t
{
  DontCare = 0,
  Off = 1,
  On = 2,
};

TRAINTASTIC_ENUM(InputCondition, "input_condition", 3,
{
  {InputCondition::DontCare, "dont_care"},
  {InputCondition::Off, "off"},
  {InputCondition::On, "on"},
});

inline constexpr std::array<InputCondition, 3> inputConditionValues{{
  InputCondition::DontCare,
  InputCondition::Off,
  InputCondition::On,
}};

#endif
