/**
 * server/src/hardware/protocol/marklincan/messages.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLINCAN_MESSAGES_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLINCAN_MESSAGES_HPP

#include <cstdint>
#include <cassert>
#include <cstring>
#include <string>
#include <initializer_list>
#include "../../../utils/byte.hpp"
#include "../../../utils/endian.hpp"

namespace MarklinCAN {

struct Message;

constexpr uint16_t calcHash(uint32_t uid)
{
  const uint16_t hash = high16(uid) ^ low16(uid);
  return ((hash << 3) & 0xFC00) | 0x0300 | (hash & 0x007F);
}

std::string toString(const Message& message);

enum class Command : uint8_t
{
  System = 0x00,
  Discovery = 0x01,
  Bind = 0x02,
  Verify = 0x03,
  LocomotiveSpeed = 0x04,
  LocomotiveDirection = 0x05,
  LocomotiveFunction = 0x06,
  ReadConfig = 0x07,
  WriteConfig = 0x08,
  AccessoryControl = 0x0B,
  Ping = 0x18,
};

enum class SystemSubCommand : uint8_t
{
  SystemStop = 0x00,
  SystemGo = 0x01,
  SystemHalt = 0x02,
  LocomotiveEmergencyStop = 0x03,
  LocomotiveCycleEnd = 0x04,
  //Neuanmeldezahler = 0x09,
  Overload = 0x0A,
  Status = 0x0B,
};

struct Message
{
  uint32_t id = 0;
  uint8_t dlc = 0;
  uint8_t data[8] = {0, 0, 0, 0, 0, 0, 0, 0};

  Message() = default;

  Message(Command command, bool response, uint32_t uid = 0)
  {
    id = (static_cast<uint32_t>(command) << 17) | (response ? 0x00010000 : 0) | calcHash(uid);
  }

  Message(Command command, bool response, std::initializer_list<uint8_t> data_, uint32_t uid = 0)
    : Message(command, response, uid)
  {
    assert(data_.size() <= sizeof(data));
    dlc = data_.size();
    if(data_.size() != 0)
      std::memcpy(data, data_.begin(), data_.size());
  }

  uint8_t priority() const
  {
    return (id >> 25) & 0x0F;
  }

  Command command() const
  {
    return static_cast<Command>((id >> 17) & 0xFF);
  }

  bool isResponse() const
  {
    return (id >> 16) & 0x001;
  }

  uint16_t hash() const
  {
    return id & 0xFFFF;
  }
};

struct SystemMessage : Message
{
  SystemMessage(SystemSubCommand subCommand, uint32_t uid = 0)
    : Message{Command::System, false, uid}
  {
    dlc = 5;
    *reinterpret_cast<uint32_t*>(&data[0]) = host_to_be(uid);
    data[4] = static_cast<uint8_t>(subCommand);
  }

  uint32_t uid() const
  {
    return be_to_host(*reinterpret_cast<const uint32_t*>(&data[0]));
  }

  SystemSubCommand subCommand() const
  {
    return static_cast<SystemSubCommand>(data[4]);
  }
};

struct SystemStop : SystemMessage
{
  SystemStop(uint32_t uid = 0)
    : SystemMessage(SystemSubCommand::SystemStop, uid)
  {
  }
};

struct SystemGo : SystemMessage
{
  SystemGo(uint32_t uid = 0)
    : SystemMessage(SystemSubCommand::SystemGo, uid)
  {
  }
};

struct SystemHalt : SystemMessage
{
  SystemHalt(uint32_t uid = 0)
    : SystemMessage(SystemSubCommand::SystemHalt, uid)
  {
  }
};

}

#endif