/**
 * shared/src/traintastic/enum/outputpairvalue.hpp
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_OUTPUTPAIRVALUE_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_OUTPUTPAIRVALUE_HPP

#include <cstdint>
#include <array>
#include "enum.hpp"

enum class OutputPairValue : uint8_t
{
  Undefined = 0,
  First = 1,
  Second = 2,
};

TRAINTASTIC_ENUM(OutputPairValue, "output_pair_value", 3,
{
  {OutputPairValue::Undefined, "undefined"},
  {OutputPairValue::First, "first"},
  {OutputPairValue::Second, "second"},
});

inline constexpr std::array<OutputPairValue, 3> outputPairValueValues{{
  OutputPairValue::Undefined,
  OutputPairValue::First,
  OutputPairValue::Second,
}};

#endif
