/**
 * shared/src/enum/decoderfunctiontype.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_DECODERFUNCTIONTYPE_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_DECODERFUNCTIONTYPE_HPP

#include <cstdint>
#include <array>
#include "enum.hpp"

enum class DecoderFunctionType : uint8_t
{
  OnOff = 0,
  Momentary = 1,
  Hold = 2,
  AlwaysOff = 3,
  AlwaysOn = 4,
};

TRAINTASTIC_ENUM(DecoderFunctionType, "decoder_function_type", 5,
{
  {DecoderFunctionType::OnOff, "on_off"},
  {DecoderFunctionType::Momentary, "momentary"},
  {DecoderFunctionType::Hold, "hold"},
  {DecoderFunctionType::AlwaysOff, "always_off"},
  {DecoderFunctionType::AlwaysOn, "always_on"},
});

inline constexpr std::array<DecoderFunctionType, 5> decoderFunctionTypeValues{{
  DecoderFunctionType::OnOff,
  DecoderFunctionType::Momentary,
  DecoderFunctionType::Hold,
  DecoderFunctionType::AlwaysOff,
  DecoderFunctionType::AlwaysOn,
}};

#endif
