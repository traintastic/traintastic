/**
 * server/src/hardware/protocol/traintasticcs/messages.cpp
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

#include "messages.hpp"
#include <cassert>
#include <string_view>
#include "../../../utils/tohex.hpp"

namespace TraintasticCS {

std::string toString(const Message& message)
{
  std::string s{toString(message.command)};

  switch(message.command)
  {
    case Command::Reset:
    case Command::Ping:
    case Command::GetInfo:
    case Command::InitXpressNet:
    case Command::ResetOk:
    case Command::Pong:
    case Command::InitXpressNetOk:
    case Command::InitS88Ok:
      assert(message.length == 0);
      break;

    case Command::Info:
    {
      const auto& info = static_cast<const Info&>(message);
      s.append(" board=")
        .append(::toString(info.board))
        .append(" version=")
        .append(std::to_string(info.versionMajor))
        .append(".")
        .append(std::to_string(info.versionMinor))
        .append(".")
        .append(std::to_string(info.versionPatch));
      break;
    }
    case Command::InitS88:
    {
      const auto& initS88 = static_cast<const InitS88&>(message);
      s.append(" moduleCount=").append(std::to_string(initS88.moduleCount))
        .append(" clockFrequency=").append(std::to_string(initS88.clockFrequency));
      break;
    }
    case Command::InputStateChanged:
    {
      const auto& inputStateChanged = static_cast<const InputStateChanged&>(message);
      s.append(" channel=").append(::toString(inputStateChanged.channel))
        .append(" address=").append(std::to_string(inputStateChanged.address()))
        .append(" state=").append(::toString(inputStateChanged.state));
      break;
    }
    case Command::ThrottleSetSpeedDirection:
    {
      const auto& setSpeedDirection = static_cast<const ThrottleSetSpeedDirection&>(message);
      s.append(" channel=").append(::toString(setSpeedDirection.channel))
        .append(" throttleId=").append(std::to_string(setSpeedDirection.throttleId()))
        .append(" protocol=").append(EnumValues<DecoderProtocol>::value.at(setSpeedDirection.protocol()))
        .append(" address=").append(std::to_string(setSpeedDirection.address()));
      if(setSpeedDirection.eStop)
      {
        s.append(" eStop");
      }
      if(setSpeedDirection.setSpeedStep)
      {
        s.append(" speedStep=").append(std::to_string(setSpeedDirection.speedStep)).append("/").append(std::to_string(setSpeedDirection.speedSteps));
      }
      if(setSpeedDirection.setDirection)
      {
        s.append(" direction=").append(setSpeedDirection.direction ? "fwd" : "rev");
      }
      break;
    }
    case Command::ThrottleSetFunctions:
    {
      const auto& setFunctions = static_cast<const ThrottleSetFunctions&>(message);
      s.append(" channel=").append(::toString(setFunctions.channel))
        .append(" throttleId=").append(std::to_string(setFunctions.throttleId()))
        .append(" protocol=").append(EnumValues<DecoderProtocol>::value.at(setFunctions.protocol()))
        .append(" address=").append(std::to_string(setFunctions.address()));

      for(uint8_t i = 0; i < setFunctions.functionCount(); ++i)
      {
        const auto function = setFunctions.function(i);
        s.append(" f").append(std::to_string(function.first)).append("=").append(function.second ? "on" : "off");
      }
      break;
    }
    case Command::Error:
    {
      const auto& error = static_cast<const Error&>(message);
      s.append(" request=").append(toString(error.request))
        .append(" code=").append(toString(error.code));
      break;
    }
  }

  s.append(" [")
    .append(toHex(reinterpret_cast<const void*>(&message), message.size(), true))
    .append("]");

  return s;
}

}
