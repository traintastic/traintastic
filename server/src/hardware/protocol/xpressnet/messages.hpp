/**
 * server/src/hardware/protocol/xpressnet/messages.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_XPRESSNET_MESSAGES_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_XPRESSNET_MESSAGES_HPP

#include <cstdint>
#include <cassert>
#include <string>
#include "../../../enum/direction.hpp"

namespace XpressNet {

struct Message;

uint8_t calcChecksum(const Message& msg);
bool isChecksumValid(const Message& msg);
std::string to_string(const Message& message, bool raw = false);

struct Message
{
  uint8_t header;

  uint8_t identification() const
  {
    return header & 0xF0;
  }

  uint8_t dataSize() const
  {
    return header & 0x0F;
  }

  uint8_t size() const
  {
    return 2 + dataSize();
  }
};
static_assert(sizeof(Message) == 1);

struct NormalOperationResumed : Message
{
  uint8_t db1 = 0x01;
  uint8_t checksum = 0x60;

  NormalOperationResumed()
  {
    header = 0x61;
  }
};
static_assert(sizeof(NormalOperationResumed) == 3);

struct TrackPowerOff : Message
{
  uint8_t db1 = 0x00;
  uint8_t checksum = 0x61;

  TrackPowerOff()
  {
    header = 0x61;
  }
};
static_assert(sizeof(TrackPowerOff) == 3);

struct EmergencyStop : Message
{
  uint8_t db1 = 0x00;
  uint8_t checksum = 0x81;

  EmergencyStop()
  {
    header = 0x81;
  }
};
static_assert(sizeof(EmergencyStop) == 3);

struct EmergencyStopLocomotive : Message
{
  uint8_t addressHigh;
  uint8_t addressLow;
  uint8_t checksum;

  EmergencyStopLocomotive(uint16_t address, bool longAddress)
  {
    header = 0x92;
    if(longAddress)
    {
      assert(address >= 1 && address <= 9999);
      addressHigh = 0x00;
      addressLow = address & 0x7f;
    }
    else
    {
      assert(address >= 1 && address <= 127);
      addressHigh = 0x00;
      addressLow = address & 0x7f;
    }
  }
};
static_assert(sizeof(EmergencyStopLocomotive) == 4);

struct LocomotiveInstruction : Message
{
  uint8_t identification;
  uint8_t addressHigh;
  uint8_t addressLow;

  LocomotiveInstruction(uint16_t address, bool longAddress)
  {
    header = 0xE4;
    if(longAddress)
    {
      assert(address >= 1 && address <= 9999);
      addressHigh = 0xc0 | address >> 8;
      addressLow = address & 0xff;
    }
    else
    {
      assert(address >= 1 && address <= 127);
      addressHigh = 0x00;
      addressLow = address & 0x7f;
    }
  }
};
static_assert(sizeof(LocomotiveInstruction) == 4);

struct SpeedAndDirectionInstruction : LocomotiveInstruction
{
  uint8_t speedAndDirection;
  uint8_t checksum;

  SpeedAndDirectionInstruction(uint16_t address, bool longAddress, bool emergencyStop, Direction direction) :
    LocomotiveInstruction(address, longAddress)
  {
    speedAndDirection = emergencyStop ? 0x01 : 0x00;
    if(direction == Direction::Forward)
      speedAndDirection |= 0x80;
  }
};

struct SpeedAndDirectionInstruction14 : SpeedAndDirectionInstruction
{
  SpeedAndDirectionInstruction14(uint16_t address, bool longAddress, bool emergencyStop, Direction direction, uint8_t speedStep, bool fl) :
    SpeedAndDirectionInstruction(address, longAddress, emergencyStop, direction)
  {
    assert(speedStep <= 14);
    identification = 0x10;
    if(!emergencyStop && speedStep > 0)
      speedAndDirection |= speedStep + 1;
    if(fl)
      speedAndDirection |= 0x10;
    checksum = calcChecksum(*this);
  }
};

struct SpeedAndDirectionInstruction27 : SpeedAndDirectionInstruction
{
  SpeedAndDirectionInstruction27(uint16_t address, bool longAddress, bool emergencyStop, Direction direction, uint8_t speedStep) :
    SpeedAndDirectionInstruction(address, longAddress, emergencyStop, direction)
  {
    assert(speedStep <= 27);
    identification = 0x11;
    if(!emergencyStop && speedStep > 0)
      speedAndDirection |= (((speedStep + 1) & 0x01) << 4) | ((speedStep + 1) >> 1);
    checksum = calcChecksum(*this);
  }
};

struct SpeedAndDirectionInstruction28 : SpeedAndDirectionInstruction
{
  SpeedAndDirectionInstruction28(uint16_t address, bool longAddress, bool emergencyStop, Direction direction, uint8_t speedStep) :
    SpeedAndDirectionInstruction(address, longAddress, emergencyStop, direction)
  {
    assert(speedStep <= 28);
    identification = 0x12;
    if(!emergencyStop && speedStep > 0)
      speedAndDirection |= (((speedStep + 1) & 0x01) << 4) | ((speedStep + 1) >> 1);
    checksum = calcChecksum(*this);
  }
};

struct SpeedAndDirectionInstruction128 : SpeedAndDirectionInstruction
{
  SpeedAndDirectionInstruction128(uint16_t address, bool longAddress, bool emergencyStop, Direction direction, uint8_t speedStep) :
    SpeedAndDirectionInstruction(address, longAddress, emergencyStop, direction)
  {
    assert(speedStep <= 126);
    identification = 0x13;
    if(!emergencyStop && speedStep > 0)
      speedAndDirection |= speedStep + 1;
    checksum = calcChecksum(*this);
  }
};

struct FunctionInstructionGroup : LocomotiveInstruction
{
  uint8_t functions = 0x00;
  uint8_t checksum;

  FunctionInstructionGroup(uint16_t address, bool longAddress, uint8_t group) :
    LocomotiveInstruction(address, longAddress)
  {
    assert(group >= 1 && group <= 3);
    identification = 0x1F + group;
  }
};

struct FunctionInstructionGroup1 : FunctionInstructionGroup
{
  FunctionInstructionGroup1(uint16_t address, bool longAddress, bool f0, bool f1, bool f2, bool f3, bool f4) :
    FunctionInstructionGroup(address, longAddress, 1)
  {
    if(f0)
      functions |= 0x10;
    if(f1)
      functions |= 0x01;
    if(f2)
      functions |= 0x02;
    if(f3)
      functions |= 0x04;
    if(f4)
      functions |= 0x08;

    checksum = calcChecksum(*this);
  }
};

struct FunctionInstructionGroup2 : FunctionInstructionGroup
{
  FunctionInstructionGroup2(uint16_t address, bool longAddress, bool f5, bool f6, bool f7, bool f8) :
    FunctionInstructionGroup(address, longAddress, 2)
  {
    if(f5)
      functions |= 0x01;
    if(f6)
      functions |= 0x02;
    if(f7)
      functions |= 0x04;
    if(f8)
      functions |= 0x08;

    checksum = calcChecksum(*this);
  }
};

struct FunctionInstructionGroup3 : FunctionInstructionGroup
{
  FunctionInstructionGroup3(uint16_t address, bool longAddress, bool f9, bool f10, bool f11, bool f12) :
    FunctionInstructionGroup(address, longAddress, 3)
  {
    if(f9)
      functions |= 0x01;
    if(f10)
      functions |= 0x02;
    if(f11)
      functions |= 0x04;
    if(f12)
      functions |= 0x08;

    checksum = calcChecksum(*this);
  }
};

/*
struct setFunctionStateGroup : LocomotiveInstruction
{
  uint8_t state = 0x00;
  uint8_t checksum;

  setFunctionStateGroup(uint8_t group, uint16_t address)
  {
    assert(group >= 1 && group <= 3);
    header = 0xE4;
    identification = 0x23 + group;
    addressLowHigh(address, addressLow, addressHigh);
  }
} __attribute__((packed));
*/

struct RocoFunctionInstructionF13F20 : LocomotiveInstruction
{
  uint8_t functions = 0x00;
  uint8_t checksum;

  RocoFunctionInstructionF13F20(uint16_t address, bool longAddress, bool f13, bool f14, bool f15, bool f16, bool f17, bool f18, bool f19, bool f20) :
    LocomotiveInstruction(address, longAddress)
  {
    identification = 0xF3;

    if(f13)
      functions |= 0x01;
    if(f14)
      functions |= 0x02;
    if(f15)
      functions |= 0x04;
    if(f16)
      functions |= 0x08;
    if(f17)
      functions |= 0x10;
    if(f18)
      functions |= 0x20;
    if(f19)
      functions |= 0x40;
    if(f20)
      functions |= 0x80;

    checksum = calcChecksum(*this);
  }
};

}

inline bool operator ==(const XpressNet::Message& lhs, const XpressNet::Message& rhs)
{
  return lhs.size() == rhs.size() && std::memcmp(&lhs, &rhs, lhs.size()) == 0;
}

#endif
