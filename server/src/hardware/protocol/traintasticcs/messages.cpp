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

constexpr std::string_view toString(Command value)
{
  switch(value)
  {
    case Command::Ping:
      return "Ping";
    case Command::GetInfo:
      return "GetInfo";
    case Command::Pong:
      return "Pong";
    case Command::Info:
      return "Info";
  }
  return {};
}

std::string toString(const Message& message)
{
  std::string s{toString(message.command)};

  switch(message.command)
  {
    case Command::Ping:
    case Command::GetInfo:
    case Command::Pong:
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
  }

  s.append(" [")
    .append(toHex(reinterpret_cast<const void*>(&message), message.size(), true))
    .append("]");

  return s;
}

}
