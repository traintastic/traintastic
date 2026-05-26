/**
 * shared/src/enum/decoderfunctionfunction.hpp
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_DECODERFUNCTIONFUNCTION_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_DECODERFUNCTIONFUNCTION_HPP

#include <cstdint>
#include <array>
#include "enum.hpp"

enum class DecoderFunctionFunction : uint16_t
{
  Generic = 0,
  Light = 1,
  Sound = 2,
  Mute = 3,
  Smoke = 4,
};

TRAINTASTIC_ENUM(DecoderFunctionFunction, "decoder_function_function", 5,
{
  {DecoderFunctionFunction::Generic, "generic"},
  {DecoderFunctionFunction::Light, "light"},
  {DecoderFunctionFunction::Sound, "sound"},
  {DecoderFunctionFunction::Mute, "mute"},
  {DecoderFunctionFunction::Smoke, "smoke"},
});

inline constexpr std::array<DecoderFunctionFunction, 5> decoderFunctionFunctionValues{{
  DecoderFunctionFunction::Generic,
  DecoderFunctionFunction::Light,
  DecoderFunctionFunction::Sound,
  DecoderFunctionFunction::Mute,
  DecoderFunctionFunction::Smoke,
}};

#endif
