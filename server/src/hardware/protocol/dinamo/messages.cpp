/**
 * server/src/hardware/protocol/dinamo/messages.cpp
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
#include "../../../utils/tohex.hpp"

namespace Dinamo {

Checksum calcChecksum(const Message& message)
{
  const Checksum* p = reinterpret_cast<const Checksum*>(&message);
  Checksum checksum = p[0];
  for(Checksum i = 1; i < message.size() - 1; i++)
  {
    checksum += p[1];
  }
  return (0x80 | (~checksum + 1));
}

void updateChecksum(Message& message)
{
  *(reinterpret_cast<Checksum*>(&message) + message.size() - 1) = calcChecksum(message);
}

bool isChecksumValid(const Message& message)
{
  const Checksum checksum = reinterpret_cast<const Checksum*>(&message)[message.size() - 1];;
  return calcChecksum(message) == checksum;
}

bool isValid(const Message& message)
{
  const uint8_t* p = reinterpret_cast<const uint8_t*>(&message);
  if((p[0] & 0x80) != 0x00)
  {
    return false;
  }
  for(uint8_t i = 1; i < message.size(); i++)
  {
    if((p[i] & 0x80) != 0x80)
    {
      return false;
    }
  }
  return true;
}

std::string toString(const Message& message)
{
  std::string s;

  if(message.header.dataSize() >= 1)
  {
    const auto command = static_cast<const Command&>(message).command;
    s.append(" command=0x").append(toHex<uint8_t>(command & 0x7F));
    switch(command)
    {
      case Command::systemControl:
        if(message.header.dataSize() >= 2)
        {
          const auto subCommand = static_cast<const SubCommand&>(message).subCommand;
          s.append(" subCommand=0x").append(toHex<uint8_t>(subCommand & 0x7F));
          switch(subCommand)
          {
            case SubCommand::protocolVersion:
            {
              if(message.size() == sizeof(ProtocolVersionRequest))
              {
                s.insert(0, "ProtocolVersionRequest");
              }
              else if(message.size() == sizeof(ProtocolVersionResponse))
              {
                const auto& response = static_cast<const ProtocolVersionResponse&>(message);
                s.insert(0, "VersionInfoResponse")
                  .append(" majorRelease=").append(std::to_string(response.majorRelease()))
                  .append(" minorRelease=").append(std::to_string(response.minorRelease()))
                  .append(" subRelease=").append(std::to_string(response.subRelease()))
                  .append(" bugFix=").append(std::to_string(response.bugFix()));
              }
              break;
            }
          }
        }
        break;
    }
  }
  else
  {
    s = "Null";
  }

  s.append(" <")
    .append(message.header.isToggleSet() ? "T" : "t")
    .append(!message.header.isJumbo() && message.header.isFault() ? "F" : " ")
    .append(!message.header.isJumbo() && message.header.isHold() ? "H" : " ")
    .append(message.header.isJumbo() ? "J" : " ")
    .append("> [")
    .append(toHex(reinterpret_cast<const void*>(&message), message.size(), true))
    .append("]");

  return s;
}

bool isNull(const Message& message)
{
  return message.header.dataSize() == 0;
}

}
