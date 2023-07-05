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
  S88Polling = 0x10,
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
  static constexpr uint32_t responseMask = 0x00010000;

  uint32_t id = 0;
  uint8_t data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  uint8_t dlc = 0;

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
    return id & responseMask;
  }

  void setResponse(bool value)
  {
    if(value)
      id |= responseMask;
    else
      id &= ~responseMask;
  }

  uint16_t hash() const
  {
    return id & 0xFFFF;
  }
};

struct UidMessage : Message
{
  UidMessage(Command command, bool response, uint32_t uid)
    : Message(command, response, uid)
  {
    dlc = 4;
    *reinterpret_cast<uint32_t*>(&data[0]) = host_to_be(uid);
  }

  uint32_t uid() const
  {
    return be_to_host(*reinterpret_cast<const uint32_t*>(&data[0]));
  }
};

struct SystemMessage : UidMessage
{
  SystemMessage(SystemSubCommand subCommand, uint32_t uid = 0)
    : UidMessage{Command::System, false, uid}
  {
    dlc = 5;
    data[4] = static_cast<uint8_t>(subCommand);
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

struct LocomotiveEmergencyStop : SystemMessage
{
  LocomotiveEmergencyStop(uint32_t uid)
    : SystemMessage(SystemSubCommand::LocomotiveEmergencyStop, uid)
  {
  }
};

struct LocomotiveSpeed : UidMessage
{
  static constexpr uint16_t speedMax = 1000;

  LocomotiveSpeed(uint32_t uid)
    : UidMessage(Command::LocomotiveSpeed, false, uid)
  {
  }

  LocomotiveSpeed(uint32_t uid, uint16_t speed_, bool response = false)
    : LocomotiveSpeed(uid)
  {
    setResponse(response);
    dlc = 6;
    setSpeed(speed_);
  }

  bool hasSpeed() const
  {
    return dlc == 6;
  }

  uint16_t speed() const
  {
    assert(hasSpeed());
    return to16(data[5], data[4]);
  }

  void setSpeed(uint16_t value)
  {
    assert(hasSpeed());
    assert(value <= speedMax);
    data[4] = high8(value);
    data[5] = low8(value);
  }
};

struct LocomotiveDirection : UidMessage
{
  enum class Direction : uint8_t
  {
    Same = 0,
    Forward = 1,
    Reverse = 2,
    Inverse = 3,
  };

  LocomotiveDirection(uint32_t uid)
    : UidMessage(Command::LocomotiveDirection, false, uid)
  {
  }

  LocomotiveDirection(uint32_t uid, Direction direction_, bool response = false)
    : LocomotiveDirection(uid)
  {
    setResponse(response);
    dlc = 5;
    setDirection(direction_);
  }

  bool hasDirection() const
  {
    return dlc == 5;
  }

  Direction direction() const
  {
    assert(hasDirection());
    return static_cast<Direction>(data[4]);
  }

  void setDirection(Direction value)
  {
    assert(hasDirection());
    data[4] = static_cast<uint8_t>(value);
  }
};

struct LocomotiveFunction : UidMessage
{
  static constexpr uint8_t numberMax = 31;
  static constexpr uint8_t valueOff = 0;
  static constexpr uint8_t valueOnMin = 1;
  static constexpr uint8_t valueOnMax = 31;

  LocomotiveFunction(uint32_t uid, uint8_t number_)
    : UidMessage(Command::LocomotiveFunction, false, uid)
  {
    dlc = 5;
    setNumber(number_);
  }

  LocomotiveFunction(uint32_t uid, uint8_t number_, uint8_t value_, bool response = false)
    : LocomotiveFunction(uid, number_)
  {
    setResponse(response);
    dlc = 6;
    setNumber(value_);
  }

  LocomotiveFunction(uint32_t uid, uint8_t number_, bool value_, bool response = false)
    : LocomotiveFunction(uid, number_)
  {
    setResponse(response);
    dlc = 6;
    setValue(value_);
  }

  uint8_t number() const
  {
    return data[4];
  }

  void setNumber(uint8_t value)
  {
    assert(value <= numberMax);
    data[4] = value;
  }

  bool hasValue() const
  {
    return dlc == 6;
  }

  bool isOn() const
  {
    return value() != valueOff;
  }

  uint8_t value() const
  {
    assert(hasValue());
    return data[5];
  }

  void setValue(bool value)
  {
    setValue(value ? valueOnMin : valueOff);
  }

  void setValue(uint8_t value)
  {
    assert(hasValue());
    assert(value <= valueOnMax);
    data[5] = value;
  }
};

struct S88ModuleCount : UidMessage
{
  S88ModuleCount(uint32_t uid, uint8_t count_)
    : UidMessage(Command::S88Polling, false, uid)
  {
    dlc = 5;
    setCount(count_);
  }

  uint8_t count() const
  {
    return data[5];
  }

  void setCount(uint8_t value)
  {
    data[5] = value;
  }
};

struct S88ModuleState : UidMessage
{
  //! \todo verify state endianess, asume big endian for now.

  S88ModuleState(uint32_t uid, uint8_t module_)
    : UidMessage(Command::S88Polling, true, uid)
  {
    dlc = 7;
    setModule(module_);
  }

  uint8_t module() const
  {
    return data[5];
  }

  void setModule(uint8_t value)
  {
    data[5] = value;
  }

  uint16_t state() const
  {
    return to16(data[7], data[6]);
  }

  void setState(uint16_t value)
  {
    data[6] = low8(value);
    data[7] = high8(value);
  }

  bool getState(uint8_t index) const
  {
    assert(index < 16);
    return state() & (1 << index);
  }

  void setState(uint8_t index, bool value)
  {
    assert(index < 16);
    if(value)
      setState(state() | (1 << index));
    else
      setState(state() & ~(1 << index));
  }
};

}

#endif