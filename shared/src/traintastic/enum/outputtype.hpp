/**
 * shared/src/traintastic/enum/outputtype.hpp
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_OUTPUTTYPE_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_OUTPUTTYPE_HPP

#include <cstdint>
#include <array>
#include "enum.hpp"


enum class OutputType : uint8_t
{
  Single = 1,
  Pair = 2,
  Aspect = 3,
  ECoSState = 4,
};

TRAINTASTIC_ENUM(OutputType, "output_type", 4,
{
  {OutputType::Single, "single"},
  {OutputType::Pair, "pair"},
  {OutputType::Aspect, "aspect"},
  {OutputType::ECoSState, "ecos_state"}
});

inline constexpr std::array<OutputType, 4> outputTypeValues{{
  OutputType::Single,
  OutputType::Pair,
  OutputType::Aspect,
  OutputType::ECoSState,
}};

#endif
