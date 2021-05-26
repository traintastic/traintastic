/**
 * server/src/enum/decoderfunctiontype.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_ENUM_DECODERFUNCTIONTYPE_HPP
#define TRAINTASTIC_SERVER_ENUM_DECODERFUNCTIONTYPE_HPP

#include <traintastic/enum/decoderfunctiontype.hpp>
#include <array>

inline constexpr std::array<DecoderFunctionType, 8> decoderFunctionTypeValues{{
  DecoderFunctionType::OnOff,
  DecoderFunctionType::Momentary,
  DecoderFunctionType::Light,
  DecoderFunctionType::Sound,
  DecoderFunctionType::Mute,
  DecoderFunctionType::Smoke,
  DecoderFunctionType::AlwaysOff,
  DecoderFunctionType::AlwaysOn,
}};

constexpr bool isAlwaysOffOrOn(DecoderFunctionType type)
{
  switch(type)
  {
    case DecoderFunctionType::AlwaysOff:
    case DecoderFunctionType::AlwaysOn:
      return true;

    case DecoderFunctionType::OnOff:
    case DecoderFunctionType::Momentary:
    case DecoderFunctionType::Light:
    case DecoderFunctionType::Sound:
    case DecoderFunctionType::Mute:
    case DecoderFunctionType::Smoke:
      break;
  }
  return false;
}

#endif
