/**
 * server/src/hardware/protocol/xpressnet/messages.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2023 Reinder Feenstra
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
#include <cstring>
#include <string>
#include "../../../enum/direction.hpp"

namespace XpressNet {

constexpr uint16_t shortAddressMin = 1;
constexpr uint16_t shortAddressMax = 99;
constexpr uint16_t longAddressMin = 100;
constexpr uint16_t longAddressMax = 9999;

constexpr uint8_t idFeedbackBroadcast = 0x40;

struct Message;

uint8_t calcChecksum(const Message& msg);
void updateChecksum(Message& msg);
bool isChecksumValid(const Message& msg);
std::string toString(const Message& message, bool raw = false);

struct Message
{
  uint8_t header;

  Message()
  {
  }

  Message(uint8_t _header) :
    header{_header}
  {
  }

  constexpr uint8_t identification() const
  {
    return header & 0xF0;
  }

  constexpr uint8_t dataSize() const
  {
    return header & 0x0F;
  }

  constexpr uint8_t size() const
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

struct FeedbackBroadcast : Message
{
  struct Pair
  {
    enum class Type
    {
      AccessoryDecoderWithoutFeedback = 0,
      AccessoryDecoderWithFeedback = 1,
      FeedbackModule = 2,
      ReservedForFutureUse = 3,
    };

    uint8_t address;
    uint8_t data;

    constexpr uint16_t groupAddress() const
    {
      return (static_cast<uint16_t>(address) << 1) | ((data & 0x10) ? 1 : 0);
    }

    void setGroupAddress(uint16_t value)
    {
      assert(value < 512);
      address = value >> 1;
      if(value & 0x0001)
        data |= 0x10;
      else
        data &= 0xEF;
    }

    constexpr bool switchingCommandCompleted() const
    {
      return (data & 0x80);
    }

    constexpr Type type() const
    {
      return static_cast<Type>((data & 0x60) >> 5);
    }

    void setType(Type value)
    {
      assert(
        value == Type::AccessoryDecoderWithoutFeedback ||
        value == Type::AccessoryDecoderWithFeedback ||
        value == Type::FeedbackModule ||
        value == Type::ReservedForFutureUse);

      data = (data & 0x9F) | (static_cast<uint8_t>(value) << 5);
    }

    constexpr uint8_t statusNibble() const
    {
      return (data & 0x0F);
    }

    void setStatus(uint8_t index, bool value)
    {
      assert(index < 4);
      if(value)
        data |= (1 << index);
      else
        data &= ~static_cast<uint8_t>(1 << index);
    }
  };
  static_assert(sizeof(Pair) == 2);

  constexpr uint8_t pairCount() const
  {
    return dataSize() / sizeof(Pair);
  }

  void setPairCount(uint8_t value)
  {
    assert(value <= 7);
    header = (header & 0xF0) | (value * 2);
  }

  const Pair& pair(uint8_t index) const
  {
    assert(index < pairCount());
    return *(reinterpret_cast<const Pair*>(&header + sizeof(header)) + index);
  }

  Pair& pair(uint8_t index)
  {
    assert(index < pairCount());
    return *(reinterpret_cast<Pair*>(&header + sizeof(header)) + index);
  }
};

struct ResumeOperationsRequest : Message
{
  uint8_t db1 = 0x81;
  uint8_t checksum = 0xA0;

  ResumeOperationsRequest()
  {
    header = 0x21;
  }
};
static_assert(sizeof(ResumeOperationsRequest) == 3);

struct StopOperationsRequest : Message
{
  uint8_t db1 = 0x80;
  uint8_t checksum = 0xA1;

  StopOperationsRequest()
  {
    header = 0x21;
  }
};
static_assert(sizeof(StopOperationsRequest) == 3);

struct StopAllLocomotivesRequest : Message
{
  uint8_t checksum = 0x80;

  StopAllLocomotivesRequest()
  {
    header = 0x80;
  }
};
static_assert(sizeof(StopAllLocomotivesRequest) == 2);

struct EmergencyStopLocomotive : Message
{
  uint8_t addressHigh;
  uint8_t addressLow;
  uint8_t checksum;

  EmergencyStopLocomotive(uint16_t address)
  {
    header = 0x92;
    if(address >= longAddressMin)
    {
      assert(address >= longAddressMin && address <= longAddressMax);
      addressHigh = 0xC0 | address >> 8;
      addressLow = address & 0xff;
    }
    else
    {
      assert(address >= shortAddressMin && address <= shortAddressMax);
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

  LocomotiveInstruction(uint16_t address)
  {
    header = 0xE4;
    if(address >= longAddressMin)
    {
      assert(address >= longAddressMin && address <= longAddressMax);
      addressHigh = 0xc0 | address >> 8;
      addressLow = address & 0xff;
    }
    else
    {
      assert(address >= shortAddressMin && address <= shortAddressMax);
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

  SpeedAndDirectionInstruction(uint16_t address, bool emergencyStop, Direction direction) :
    LocomotiveInstruction(address)
  {
    assert(direction != Direction::Unknown);
    speedAndDirection = emergencyStop ? 0x01 : 0x00;
    if(direction == Direction::Forward)
      speedAndDirection |= 0x80;
  }
};

struct SpeedAndDirectionInstruction14 : SpeedAndDirectionInstruction
{
  SpeedAndDirectionInstruction14(uint16_t address, bool emergencyStop, Direction direction, uint8_t speedStep, bool fl) :
    SpeedAndDirectionInstruction(address, emergencyStop, direction)
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
  SpeedAndDirectionInstruction27(uint16_t address, bool emergencyStop, Direction direction, uint8_t speedStep) :
    SpeedAndDirectionInstruction(address, emergencyStop, direction)
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
  SpeedAndDirectionInstruction28(uint16_t address, bool emergencyStop, Direction direction, uint8_t speedStep) :
    SpeedAndDirectionInstruction(address, emergencyStop, direction)
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
  SpeedAndDirectionInstruction128(uint16_t address, bool emergencyStop, Direction direction, uint8_t speedStep) :
    SpeedAndDirectionInstruction(address, emergencyStop, direction)
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

  FunctionInstructionGroup(uint16_t address, uint8_t group) :
    LocomotiveInstruction(address)
  {
    assert(group >= 1 && group <= 5);
    identification = (group == 5) ? 0x28 : (0x1F + group);
  }
};

struct FunctionInstructionGroup1 : FunctionInstructionGroup
{
  FunctionInstructionGroup1(uint16_t address, bool f0, bool f1, bool f2, bool f3, bool f4) :
    FunctionInstructionGroup(address, 1)
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
  FunctionInstructionGroup2(uint16_t address, bool f5, bool f6, bool f7, bool f8) :
    FunctionInstructionGroup(address, 2)
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
  FunctionInstructionGroup3(uint16_t address, bool f9, bool f10, bool f11, bool f12) :
    FunctionInstructionGroup(address, 3)
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

struct FunctionInstructionGroup4 : FunctionInstructionGroup
{
  FunctionInstructionGroup4(uint16_t address, bool f13, bool f14, bool f15, bool f16, bool f17, bool f18, bool f19, bool f20) :
    FunctionInstructionGroup(address, 4)
  {
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

struct FunctionInstructionGroup5 : FunctionInstructionGroup
{
  FunctionInstructionGroup5(uint16_t address, bool f21, bool f22, bool f23, bool f24, bool f25, bool f26, bool f27, bool f28) :
    FunctionInstructionGroup(address, 5)
  {
    if(f21)
      functions |= 0x01;
    if(f22)
      functions |= 0x02;
    if(f23)
      functions |= 0x04;
    if(f24)
      functions |= 0x08;
    if(f25)
      functions |= 0x10;
    if(f26)
      functions |= 0x20;
    if(f27)
      functions |= 0x40;
    if(f28)
      functions |= 0x80;

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

struct AccessoryDecoderOperationRequest : Message
{
  uint8_t address = 0x00;
  uint8_t data = 0x80;
  uint8_t checksum;

  AccessoryDecoderOperationRequest(uint16_t fullAddress, bool value)
    : Message(0x52)
  {
    assert(fullAddress < 2048);
    address = static_cast<uint8_t>(fullAddress >> 3);
    data |= static_cast<uint8_t>(fullAddress & 0x07);
    if(value)
      data |= 0x40;
    checksum = calcChecksum(*this);
  }
};
static_assert(sizeof(AccessoryDecoderOperationRequest) == 4);

namespace RocoMultiMAUS
{
  struct FunctionInstructionF13F20 : LocomotiveInstruction
  {
    uint8_t functions = 0x00;
    uint8_t checksum;

    FunctionInstructionF13F20(uint16_t address, bool f13, bool f14, bool f15, bool f16, bool f17, bool f18, bool f19, bool f20) :
      LocomotiveInstruction(address)
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

namespace RoSoftS88XpressNetLI
{
  struct S88StartAddress : Message
  {
    static constexpr uint8_t startAddressMin = 0;
    static constexpr uint8_t startAddressMax = 127;
    static constexpr uint8_t startAddressDefault = 64;
    static constexpr uint8_t startAddressGet = 0xFF;

    uint8_t data1;
    uint8_t startAddress;
    uint8_t checksum;

    S88StartAddress(uint8_t _startAddress = startAddressGet) :
      Message(0xF2),
      data1{0xF1},
      startAddress{_startAddress}
    {
      assert((startAddress >= startAddressMin && startAddress <= startAddressMax) || startAddress == startAddressGet);
      checksum = calcChecksum(*this);
    }
  };

  struct S88ModuleCount : Message
  {
    static constexpr uint8_t moduleCountMin = 1;
    static constexpr uint8_t moduleCountMax = 32;
    static constexpr uint8_t moduleCountDefault = 2;
    static constexpr uint8_t moduleCountGet = 0xFF;

    uint8_t data1;
    uint8_t moduleCount;
    uint8_t checksum;

    S88ModuleCount(uint8_t _moduleCount = moduleCountGet) :
      Message(0xF2),
      data1{0xF2},
      moduleCount{_moduleCount}
    {
      assert((moduleCount >= moduleCountMin && moduleCount <= moduleCountMax) || moduleCount == moduleCountGet);
      checksum = calcChecksum(*this);
    }
  };
}

}

inline bool operator ==(const XpressNet::Message& lhs, const XpressNet::Message& rhs)
{
  return lhs.size() == rhs.size() && std::memcmp(&lhs, &rhs, lhs.size()) == 0;
}

#endif
