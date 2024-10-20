/**
 * server/src/hardware/protocol/dinamo/messages.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DINAMO_MESSAGES_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DINAMO_MESSAGES_HPP

#include <cstdint>
#include <cassert>
#include <string>

namespace Dinamo {

using Checksum = uint8_t;

struct Message;

Checksum calcChecksum(const Message& message);
void updateChecksum(Message& message);
bool isChecksumValid(const Message& message);
bool isValid(const Message& message);

std::string toString(const Message& message);

bool isNull(const Message& message);

constexpr void setBit(uint8_t& value, bool set, const uint8_t bit)
{
  if(set)
  {
    value |= bit;
  }
  else // clear
  {
    value &= ~bit;
  }
}

struct Header
{
  static constexpr uint8_t dataSizeMask = 0x07;
  static constexpr uint8_t jumboBit = 0x08;
  static constexpr uint8_t holdBit = 0x10;
  static constexpr uint8_t faultBit = 0x20;
  static constexpr uint8_t toggleBit = 0x40;

  uint8_t header = 0x00;

  uint8_t dataSize() const
  {
    if(isJumbo())
    {
      return 8 + ((header & dataSizeMask) | ((header >> 1) & 0x18));
    }
    return (header & dataSizeMask);
  }

  void setDataSize(uint8_t value)
  {
    assert(value <= 39);
    setJumbo(value >= 8);
    if(isJumbo())
    {
      value -= 8;
      header &= 0xC8;
      header |= value & dataSizeMask;
      header |= (value << 1) & 0x18;
    }
    else
    {
      header &= ~ dataSizeMask;
      header |= value;
    }
  }

  bool isJumbo() const
  {
    return !(header & jumboBit);
  }

  void setJumbo(bool value)
  {
    setBit(header, !value, jumboBit); // jumbo = 0, standard = 1
  }

  bool isHold() const
  {
    assert(!isJumbo());
    return (header & holdBit);
  }

  void setHold(bool value)
  {
    assert(!isJumbo());
    setBit(header, value, holdBit);
  }

  bool isFault() const
  {
    assert(!isJumbo());
    return (header & faultBit);
  }

  void setFault(bool value)
  {
    assert(!isJumbo());
    setBit(header, value, faultBit);
  }

  bool isToggleSet() const
  {
    return (header & toggleBit);
  }

  void setToggle(bool value)
  {
    setBit(header, value, toggleBit);
  }
};

struct Message
{
  Header header;

  uint8_t size() const
  {
    return sizeof(header) + header.dataSize() + sizeof(Checksum);
  }
};

struct Null : Message
{
  Checksum checksum;

  Null()
  {
    header.setDataSize(0);
  }
};

struct Command : Message
{
  static constexpr uint8_t systemControl = 0x81;

  uint8_t command;
};

struct SubCommand : Command
{
  static constexpr uint8_t resetFault = 0x80;
  static constexpr uint8_t setHFILevel = 0x81;
  static constexpr uint8_t protocolVersion = 0x82;
  static constexpr uint8_t systemInfo = 0x83;
  static constexpr uint8_t setFlashPeriod = 0x84;
  static constexpr uint8_t setOptions = 0x85;
  //static constexpr uint8_t subsystemDebuggingOff = 0x86;
  //static constexpr uint8_t subsystemDebuggingOn = 0x87;
  static constexpr uint8_t temporaryConfiguration = 0x86;
  static constexpr uint8_t permanentConfiguration = 0x87;
  static constexpr uint8_t subsystemTransmissionError = 0x89;
  static constexpr uint8_t systemVersionRequest = 0x8A;
  static constexpr uint8_t requestOptions = 0x8D;
  static constexpr uint8_t requestActiveConfiguration = 0x8E;
  static constexpr uint8_t requestSavedConfiguration = 0x8F;
  static constexpr uint8_t eventActionConfiguration = 0xA0;
  static constexpr uint8_t clearSwitchEventAction = 0xC0;
  static constexpr uint8_t setSwitchEventAction = 0xE0;

  uint8_t subCommand;
};

struct ResetFault : SubCommand
{
  Checksum checksum;

  ResetFault()
  {
    header.setDataSize(2);
    command = systemControl;
    subCommand = resetFault;
  }
};

struct SetHFILevel : SubCommand
{
  static constexpr uint8_t levelMax = 15;

  uint8_t level;
  Checksum checksum;

  SetHFILevel(uint8_t value)
  {
    assert(level <= levelMax);
    header.setDataSize(3);
    command = systemControl;
    subCommand = setHFILevel;
    level = value;
  }
};

struct ProtocolVersionRequest : SubCommand
{
  Checksum checksum;

  ProtocolVersionRequest()
  {
    header.setDataSize(2);
    command = systemControl;
    subCommand = protocolVersion;
  }
};

struct ProtocolVersionResponse : SubCommand
{
  uint8_t majorMinorRelease;
  uint8_t subReleaseBugFix;
  Checksum checksum;

  ProtocolVersionResponse()
  {
    header.setDataSize(4);
    command = systemControl;
    subCommand = protocolVersion;
  }

  ProtocolVersionResponse(uint8_t major, uint8_t minor, uint8_t sub, uint8_t fix)
    : ProtocolVersionResponse()
  {
    majorMinorRelease = (0x80 | (major & 0x07) << 3) | (minor & 0x07);
    subReleaseBugFix = (0x80 | (sub & 0x07) << 3) | (fix & 0x07);
  }

  uint8_t majorRelease() const
  {
    return ((majorMinorRelease >> 3) & 0x07);
  }

  uint8_t minorRelease() const
  {
    return (majorMinorRelease & 0x07);
  }

  uint8_t subRelease() const
  {
    return ((subReleaseBugFix >> 3) & 0x07);
  }

  uint8_t bugFix() const
  {
    return (subReleaseBugFix & 0x07);
  }
};



struct Block : public Message
{
  uint8_t data1;
  uint8_t data2;

  Block(uint8_t command, uint8_t block_)
    : data1(0x80 | (command & 0x7E) | (block_ >> 7))
    , data2(0x80 | (block_ & 0x7F))
  {
  }

  uint8_t block() const
  {
    return ((data1 << 7) | (data2 & 0x7F));
  }
};

struct BlockAnalog : public Block
{
  uint8_t data3;

  BlockAnalog(uint8_t block_)
    : Block(0x20, block_)
    , data3{0x80}
  {
    header.setDataSize(3);
  }
};

struct BlockAnalogSetSpeed : public BlockAnalog
{
  Checksum checksum;

  BlockAnalogSetSpeed(uint8_t block_, uint8_t speed_)
    : BlockAnalog(block_)
  {
    data3 = (0xC0 | (speed_ & 0x3F));
  }

  uint8_t speed() const
  {
    return (data3 & 0x3F);
  }
};

struct BlockAnalogSetSpeedAndPolarity : public BlockAnalogSetSpeed
{
  Checksum checksum;

  BlockAnalogSetSpeedAndPolarity(uint8_t block_, uint8_t speed_, bool positivePolarity_)
    : BlockAnalogSetSpeed(block_, speed_)
  {
    data1 |= 0x02;
    setBit(data3, positivePolarity_, 0x40);
  }

  bool positivePolarity() const
  {
    return (data3 & 0x40);
  }
};

struct BlockAnalogSetHFI : public BlockAnalog
{
  Checksum checksum;

  BlockAnalogSetHFI(uint8_t block_, bool enable)
    : BlockAnalog(block_)
  {
    header.setDataSize(3);
    if(enable)
    {
      data3 |= 0x10;
    }
  }

  uint8_t enabled() const
  {
    return (data3 & 0x10);
  }
};

}

#endif
