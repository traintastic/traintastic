/**
 * server/src/hardware/decoder/decoderchangeflags.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_DECODER_DECODERCHANGEFLAGS_HPP
#define TRAINTASTIC_SERVER_HARDWARE_DECODER_DECODERCHANGEFLAGS_HPP

#include <type_traits>

enum class DecoderChangeFlags
{
  EmergencyStop = 1 << 0,
  Direction = 1 << 1,
  Throttle = 1 << 2,
  SpeedSteps = 1 << 3,
  FunctionValue = 1 << 4,
};

constexpr DecoderChangeFlags operator| (const DecoderChangeFlags& lhs, const DecoderChangeFlags& rhs)
{
  return static_cast<DecoderChangeFlags>(static_cast<std::underlying_type_t<DecoderChangeFlags>>(lhs) | static_cast<std::underlying_type_t<DecoderChangeFlags>>(rhs));
}

constexpr void operator|= (DecoderChangeFlags& lhs, const DecoderChangeFlags& rhs)
{
  lhs = lhs | rhs;
}

constexpr bool has(const DecoderChangeFlags& value, const DecoderChangeFlags& mask)
{
  return (static_cast<std::underlying_type_t<DecoderChangeFlags>>(value) & static_cast<std::underlying_type_t<DecoderChangeFlags>>(mask)) != 0;
}

#endif
