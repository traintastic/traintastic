/**
 * server/src/hardware/protocol/traintasticdiy/opcode.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_TRAINTASTICDIY_OPCODE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_TRAINTASTICDIY_OPCODE_HPP

#include <cstdint>

namespace TraintasticDIY {

enum class OpCode : uint8_t
{
  Heartbeat = 0x00,
  GetInputState = 0x12,
  SetInputState = 0x13,
  GetOutputState = 0x22,
  SetOutputState = 0x23,
  ThrottleSubUnsub = 0x34,
  ThrottleSetFunction = 0x35,
  ThrottleSetSpeedDirection = 0x37,
  GetFeatures = 0xE0,
  Features = 0xE4,
  GetInfo = 0xF0,
  Info = 0xFF,
};

}

constexpr std::string_view toString(TraintasticDIY::OpCode value)
{
  using OpCode = TraintasticDIY::OpCode;

  switch(value)
  {
    case OpCode::Heartbeat:
      return "Heartbeat";

    case OpCode::GetInputState:
      return "GetInputState";

    case OpCode::SetInputState:
      return "SetInputState";

    case OpCode::GetOutputState:
      return "GetOutputState";

    case OpCode::SetOutputState:
      return "SetOutputState";

    case OpCode::ThrottleSubUnsub:
      return "ThrottleSubUnsub";

    case OpCode::ThrottleSetFunction:
      return "ThrottleSetFunction";

    case OpCode::ThrottleSetSpeedDirection:
      return "ThrottleSetSpeedDirection";

    case OpCode::GetFeatures:
      return "GetFeatures";

    case OpCode::Features:
      return "Features";

    case OpCode::GetInfo:
      return "GetInfo";

    case OpCode::Info:
      return "Info";
  }
  return {};
}

#endif
