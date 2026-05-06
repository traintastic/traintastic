/**
 * server/src/hardware/protocol/xpressnet/messages.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2024 Reinder Feenstra
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
#include <traintastic/enum/direction.hpp>
#include "../../../utils/packed.hpp"
#include "../../../utils/endian.hpp"
#include "../../../utils/byte.hpp"
#include "../z21/utils.hpp"
#include "utils.hpp"

namespace XpressNet {

constexpr uint16_t shortAddressMin = 1;
constexpr uint16_t shortAddressMax = 99;
constexpr uint16_t longAddressMin = 100;
constexpr uint16_t longAddressMax = 9999;

constexpr uint8_t idSetSpeed14 = 0x10;
constexpr uint8_t idSetSpeed27 = 0x11;
constexpr uint8_t idSetSpeed28 = 0x12;
constexpr uint8_t idSetSpeed128 = 0x13;

constexpr uint8_t idSetFuncGroup1 = 0x20;
constexpr uint8_t idSetFuncGroup2 = 0x21;
constexpr uint8_t idSetFuncGroup3 = 0x22;
constexpr uint8_t idSetFuncGroup4 = 0x23;
constexpr uint8_t idSetFuncGroup5 = 0x28;
constexpr uint8_t idSetFuncGroup6 = 0x29;
constexpr uint8_t idSetFuncGroup7 = 0x2A;
constexpr uint8_t idSetFuncGroup8 = 0x2B;
constexpr uint8_t idCentralVersion = idSetFuncGroup2;

constexpr uint8_t idQueryFuncGroup4and5 = 0x09;
constexpr uint8_t idQueryFuncGroup6above = 0x0B;

constexpr uint8_t idFeedbackBroadcast = 0x40;

constexpr uint8_t idSetFuncGroup9 = 0x50;
constexpr uint8_t idSetFuncGroup10 = 0x51;
constexpr uint8_t idReplyFuncF13F28 = 0x52;
constexpr uint8_t idReplyFuncF29F68 = 0x53;

constexpr uint8_t idLocomotiveBusy = 0x40;
constexpr uint8_t idQueryLocoCumulative_Roco = 0xF0;
constexpr uint8_t idSetFuncGroup4_Roco = 0xF3;

enum Header : uint8_t
{
  STOP_REQUEST = 0x21,
  ACCESSORY_REPLY_OLD = 0x42,
  ACCESSORY_REPLY = 0x43,
  SET_ACCESSORY_OLD = 0x52,
  SET_ACCESSORY = 0x53,
  BC_HEADER = 0x61,
  REPLY_VERSION_2_3 = 0x62,
  REPLY_VERSION_3_0 = 0x63,
  SET_STOP_LOCO = 0x80,
  BC_STOPPED = 0x81,
  SET_STOP_LOCO_SINGLE = 0x92,
  GET_LOCO_INFO = 0xE3,
  SET_LOCO = 0xE4,
  FUNC_INFO_V4 = 0xE6,
  LOCO_INFO_CUMULATIVE = 0xE7
};

enum HardwareType : uint8_t
{
  HWT_LZ100 = 0x00,
  HWT_LZ200 = 0x01,
  HWT_DPC = 0x02,
  HWT_multiMAUS = 0x10,
  HWT_UNKNOWN = 0xFF
};

constexpr std::string_view toString(HardwareType value)
{
  switch(value)
  {
    case HWT_LZ100:
      return "Lenz LZ100";

    case HWT_LZ200:
      return "Lenz LZ200";

    case HWT_DPC:
      return "Lenz DPC (Compact und Commander)";

    case HWT_multiMAUS:
      return "ROCO multiMAUS";

    case HWT_UNKNOWN:
    default:
      break;
  }

  return {};
}

struct Message;

inline uint8_t calcChecksum(const Message& msg);
uint8_t calcChecksum(const Message& msg, const int dataSize);

void updateChecksum(Message& msg);

inline bool isChecksumValid(const Message& msg);
bool isChecksumValid(const Message& msg, const int dataSize);

std::string toString(const Message& message, bool raw = true, const PendingQuery &pendingQuery = {});

// Chapters are based on:
// Lenz Dokumentation XpressNet Version 4.0 02/2022
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

  inline void updateChecksum()
  {
    XpressNet::updateChecksum(*this);
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(Message) == 1);

// 2.4.1
struct NormalOperationResumed : Message
{
  uint8_t db1 = 0x01;
  uint8_t checksum = 0x60;

  NormalOperationResumed()
  {
    header = BC_HEADER;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(NormalOperationResumed) == 3);

// 2.4.2
struct TrackPowerOff : Message
{
  uint8_t db1 = 0x00;
  uint8_t checksum = 0x61;

  TrackPowerOff()
  {
    header = BC_HEADER;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(TrackPowerOff) == 3);

// 2.4.3
struct EmergencyStop : Message
{
  uint8_t db1 = 0x00;
  uint8_t checksum = 0x81;

  EmergencyStop()
  {
    header = BC_STOPPED;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(EmergencyStop) == 3);

// 2.7.1 Software version reply (OLD! up to Central version 2.3)
struct CentralVersionReplyOLD : Message
{
  uint8_t db1 = idCentralVersion;
  uint8_t versionHex = 0x00;
  uint8_t checksum = 0;

  CentralVersionReplyOLD(uint8_t versionHex_)
  {
    header = REPLY_VERSION_2_3;
    versionHex = versionHex_;
    updateChecksum();
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(CentralVersionReplyOLD) == 4);

// 2.7.2 Software version reply (from Central version 3.0)
struct CentralVersionReplyV3 : Message
{
  uint8_t db1 = idCentralVersion;
  uint8_t versionHex = 0x00;
  uint8_t db_csId = 0x00;
  uint8_t checksum = 0;

  CentralVersionReplyV3(uint8_t versionHex_, uint8_t csId_)
  {
    header = REPLY_VERSION_3_0;
    versionHex = versionHex_;
    db_csId = csId_;
    updateChecksum();
  }

  HardwareType commandStationId() const
  {
    return HardwareType(db_csId);
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(CentralVersionReplyV3) == 5);

// 2.13
struct CommandStationBusy : Message
{
  uint8_t db1 = 0x81;
  uint8_t checksum = 0xE0;

  CommandStationBusy()
  {
    header = BC_HEADER;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(CommandStationBusy) == 3);

// 2.14
struct CommandUnknown : Message
{
  uint8_t db1 = 0x82;
  uint8_t checksum = 0xE3;

  CommandUnknown()
  {
    header = BC_HEADER;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(CommandUnknown) == 3);

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
  } ATTRIBUTE_PACKED;
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
} ATTRIBUTE_PACKED;

// 3.2
struct ResumeOperationsRequest : Message
{
  uint8_t db1 = 0x81;
  uint8_t checksum = 0xA0;

  ResumeOperationsRequest()
  {
    header = STOP_REQUEST;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(ResumeOperationsRequest) == 3);

// 3.3
struct StopOperationsRequest : Message
{
  uint8_t db1 = 0x80;
  uint8_t checksum = 0xA1;

  StopOperationsRequest()
  {
    header = STOP_REQUEST;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(StopOperationsRequest) == 3);

// 3.4
struct StopAllLocomotivesRequest : Message
{
  uint8_t checksum = 0x80;

  StopAllLocomotivesRequest()
  {
    header = SET_STOP_LOCO;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(StopAllLocomotivesRequest) == 2);

// 3.7 Emergency stop locomotive (from Central version 3.0)
struct EmergencyStopLocomotive : Message
{
  uint8_t addressHigh;
  uint8_t addressLow;
  uint8_t checksum;

  EmergencyStopLocomotive(uint16_t address_)
  {
    header = SET_STOP_LOCO_SINGLE;
    setAddress(address_);
  }

  void setAddress(uint16_t address_)
  {
    if(address_ >= longAddressMin)
    {
      assert(address_ >= longAddressMin && address_ <= longAddressMax);
      addressHigh = 0xC0 | high8(address_);
      addressLow = low8(address_);
    }
    else
    {
      assert(address_ >= shortAddressMin && address_ <= shortAddressMax);
      addressHigh = 0x00;
      addressLow = address_ & 0x7F;
    }
  }

  inline uint16_t address() const
  {
    return to16(addressLow, addressHigh & 0x3F);
  }

  inline bool isLongAddress() const
  {
    return (addressHigh & 0xC0) == 0xC0;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(EmergencyStopLocomotive) == 4);

struct LocomotiveInstruction : Message
{
  uint8_t identification;
  uint8_t addressHigh;
  uint8_t addressLow;

  LocomotiveInstruction(uint16_t address_)
  {
    header = SET_LOCO;
    setAddress(address_);
  }

  void setAddress(uint16_t address_)
  {
    if(address_ >= longAddressMin)
    {
      assert(address_ >= longAddressMin && address_ <= longAddressMax);
      addressHigh = 0xC0 | high8(address_);
      addressLow = low8(address_);
    }
    else
    {
      assert(address_ >= shortAddressMin && address_ <= shortAddressMax);
      addressHigh = 0x00;
      addressLow = address_ & 0x7F;
    }
  }

  inline uint16_t address() const
  {
    return to16(addressLow, addressHigh & 0x3F);
  }

  inline bool isLongAddress() const
  {
    return (addressHigh & 0xC0) == 0xC0;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LocomotiveInstruction) == 4);

// 3.41.3 Drive locomotive 14/27/28/128 (from Central version 3.0)
struct SpeedAndDirectionInstruction : LocomotiveInstruction
{
  static constexpr uint8_t directionBit = 0x80;
  static constexpr uint8_t stop = 0x00;
  static constexpr uint8_t eStop = 0x01;

  uint8_t speedAndDirection;
  uint8_t checksum;

  SpeedAndDirectionInstruction(uint16_t address, bool emergencyStop, Direction dir) :
    LocomotiveInstruction(address)
  {
    assert(dir != Direction::Unknown);
    speedAndDirection = emergencyStop ? eStop : stop;
    if(dir == Direction::Forward)
      speedAndDirection |= directionBit;
  }

  [[nodiscard]] uint8_t speedSteps() const
  {
    switch(identification)
    {
    case idSetSpeed14:
      return 14;
    case idSetSpeed27:
      return 27;
    case idSetSpeed28:
      return 28;
    case idSetSpeed128:
      return 126;
    default:
      break;
    }
    return 0;
  }

  inline void setSpeedSteps(uint8_t steps)
  {
    switch(steps)
    {
    case 14:
      identification = idSetSpeed14;
      break;
    case 27:
      identification = idSetSpeed27;
      break;
    case 28:
      identification = idSetSpeed28;
      break;
    case 126:
    case 128:
    default:
      identification = idSetSpeed128;
      break;
    }
  }

  [[nodiscard]] inline Direction direction() const
  {
    return Z21::Utils::getDirection(speedAndDirection);
  }

  inline void setDirection(Direction value)
  {
    Z21::Utils::setDirection(speedAndDirection, value);
  }

  [[nodiscard]] inline bool isEmergencyStop() const
  {
    return Z21::Utils::isEmergencyStop(speedAndDirection, speedSteps());
  }

  inline void setEmergencyStop()
  {
    Z21::Utils::setEmergencyStop(speedAndDirection);
  }

  [[nodiscard]] inline uint8_t speedStep() const
  {
    return Z21::Utils::getSpeedStep(speedAndDirection, speedSteps());
  }

  inline void setSpeedStep(uint8_t value)
  {
    Z21::Utils::setSpeedStep(speedAndDirection, speedSteps(), value);
  }

} ATTRIBUTE_PACKED;

struct SpeedAndDirectionInstruction14 : SpeedAndDirectionInstruction
{
  static constexpr uint8_t flagF0 = 0x10;

  SpeedAndDirectionInstruction14(uint16_t address, bool emergencyStop, Direction direction, uint8_t speedStep, bool fl) :
    SpeedAndDirectionInstruction(address, emergencyStop, direction)
  {
    assert(speedStep <= 14);
    identification = idSetSpeed14;
    if(!emergencyStop && speedStep > 0)
      speedAndDirection |= speedStep + 1;
    setFl(fl);
    checksum = calcChecksum(*this);
  }

  [[nodiscard]] inline bool getFl() const
  {
    return (speedAndDirection & flagF0) == flagF0;
  }

  inline void setFl(bool value)
  {
    if(value)
      speedAndDirection |= flagF0;
    else
      speedAndDirection &= ~flagF0;
  }
} ATTRIBUTE_PACKED;

struct SpeedAndDirectionInstruction27 : SpeedAndDirectionInstruction
{
  SpeedAndDirectionInstruction27(uint16_t address, bool emergencyStop, Direction direction, uint8_t speedStep) :
    SpeedAndDirectionInstruction(address, emergencyStop, direction)
  {
    assert(speedStep <= 27);
    identification = idSetSpeed27;
    if(!emergencyStop && speedStep > 0)
      speedAndDirection |= (((speedStep + 1) & 0x01) << 4) | ((speedStep + 1) >> 1);
    checksum = calcChecksum(*this);
  }
} ATTRIBUTE_PACKED;

struct SpeedAndDirectionInstruction28 : SpeedAndDirectionInstruction
{
  SpeedAndDirectionInstruction28(uint16_t address, bool emergencyStop, Direction direction, uint8_t speedStep) :
    SpeedAndDirectionInstruction(address, emergencyStop, direction)
  {
    assert(speedStep <= 28);
    identification = idSetSpeed28;
    if(!emergencyStop && speedStep > 0)
      speedAndDirection |= (((speedStep + 1) & 0x01) << 4) | ((speedStep + 1) >> 1);
    checksum = calcChecksum(*this);
  }
} ATTRIBUTE_PACKED;

struct SpeedAndDirectionInstruction128 : SpeedAndDirectionInstruction
{
  SpeedAndDirectionInstruction128(uint16_t address, bool emergencyStop, Direction direction, uint8_t speedStep) :
    SpeedAndDirectionInstruction(address, emergencyStop, direction)
  {
    assert(speedStep <= 126);
    identification = idSetSpeed128;
    if(!emergencyStop && speedStep > 0)
      speedAndDirection |= speedStep + 1;
    checksum = calcChecksum(*this);
  }
} ATTRIBUTE_PACKED;

// 3.41.4 Set locomotive function
struct FunctionInstructionGroup : LocomotiveInstruction
{
  uint8_t functions = 0x00;
  uint8_t checksum;

  FunctionInstructionGroup(uint16_t address, uint8_t group) :
    LocomotiveInstruction(address)
  {
    assert(group >= 1 && group <= 10);
    if(group < 5)
      identification = idSetFuncGroup1 + (group - 1);
    else if(group < 8)
      identification = idSetFuncGroup5 + (group - 5);
    else
      identification = idSetFuncGroup9 + (group - 9);
  }

  uint8_t getGroup() const
  {
    if(identification < idSetFuncGroup1)
      return 0;
    if(identification <= idSetFuncGroup4)
      return identification - idSetFuncGroup1 + 1;

    if(identification < idSetFuncGroup5)
      return 0;
    if(identification <= idSetFuncGroup8)
      return identification - idSetFuncGroup5 + 5;

    if(identification < idSetFuncGroup9)
      return 0;
    if(identification <= idSetFuncGroup10)
      return identification - idSetFuncGroup9 + 9;

    return 0;
  }

  static uint8_t getMaxFunctionIndex(uint8_t group)
  {
    assert(group >= 1 && group <= 10);
    if(group <= 3)
      return group * 4;
    return (group - 4) * 8 + 20;
  }

  static uint8_t getMinFunctionIndex(uint8_t group)
  {
    assert(group >= 1 && group <= 10);
    if(group == 1)
      return 0;
    return getMaxFunctionIndex(group - 1) + 1;
    return 0;
  }

  bool getFunction(uint8_t index) const
  {
    const uint8_t group = getGroup();
    const uint8_t minIndex = getMinFunctionIndex(group);
    assert(minIndex <= index);
    assert(getMaxFunctionIndex(group) >= index);

    if(group == 1)
    {
      if(index == 0)
        return (functions & 0x10) == 0x10;
      return (functions >> (index - minIndex - 1) & 0x01);
    }
    return (functions >> (index - minIndex) & 0x01);
  }

  void setFunction(uint8_t index, bool value)
  {
    const uint8_t group = getGroup();
    const uint8_t minIndex = getMinFunctionIndex(group);
    assert(minIndex <= index);
    assert(getMaxFunctionIndex(group) >= index);

    uint8_t flagBit = 0;
    if(group == 1)
    {
      if(index == 0)
        flagBit = 0x10;
      else
        flagBit = 1 << (index - minIndex - 1);
    }
    else
    {
      flagBit = 1 << (index - minIndex);
    }

    if(value)
      functions |= flagBit;
    else
      functions &= ~flagBit;
  }
} ATTRIBUTE_PACKED;

// (from Central version 3.0)
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
} ATTRIBUTE_PACKED;

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
} ATTRIBUTE_PACKED;

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
} ATTRIBUTE_PACKED;

// (from Central version 3.6)
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
} ATTRIBUTE_PACKED;

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
} ATTRIBUTE_PACKED;

// (from Central version 4.0)
struct FunctionInstructionGroup6 : FunctionInstructionGroup
{
  FunctionInstructionGroup6(uint16_t address, bool f29, bool f30, bool f31, bool f32, bool f33, bool f34, bool f35, bool f36) :
    FunctionInstructionGroup(address, 6)
  {
    if(f29)
      functions |= 0x01;
    if(f30)
      functions |= 0x02;
    if(f31)
      functions |= 0x04;
    if(f32)
      functions |= 0x08;
    if(f33)
      functions |= 0x10;
    if(f34)
      functions |= 0x20;
    if(f35)
      functions |= 0x40;
    if(f36)
      functions |= 0x80;

    checksum = calcChecksum(*this);
  }
} ATTRIBUTE_PACKED;

struct FunctionInstructionGroup7 : FunctionInstructionGroup
{
  FunctionInstructionGroup7(uint16_t address, bool f37, bool f38, bool f39, bool f40, bool f41, bool f42, bool f43, bool f44) :
    FunctionInstructionGroup(address, 7)
  {
    if(f37)
      functions |= 0x01;
    if(f38)
      functions |= 0x02;
    if(f39)
      functions |= 0x04;
    if(f40)
      functions |= 0x08;
    if(f41)
      functions |= 0x10;
    if(f42)
      functions |= 0x20;
    if(f43)
      functions |= 0x40;
    if(f44)
      functions |= 0x80;

    checksum = calcChecksum(*this);
  }
} ATTRIBUTE_PACKED;

struct FunctionInstructionGroup8 : FunctionInstructionGroup
{
  FunctionInstructionGroup8(uint16_t address, bool f45, bool f46, bool f47, bool f48, bool f49, bool f50, bool f51, bool f52) :
    FunctionInstructionGroup(address, 8)
  {
    if(f45)
      functions |= 0x01;
    if(f46)
      functions |= 0x02;
    if(f47)
      functions |= 0x04;
    if(f48)
      functions |= 0x08;
    if(f49)
      functions |= 0x10;
    if(f50)
      functions |= 0x20;
    if(f51)
      functions |= 0x40;
    if(f52)
      functions |= 0x80;

    checksum = calcChecksum(*this);
  }
} ATTRIBUTE_PACKED;

struct FunctionInstructionGroup9 : FunctionInstructionGroup
{
  FunctionInstructionGroup9(uint16_t address, bool f53, bool f54, bool f55, bool f56, bool f57, bool f58, bool f59, bool f60) :
    FunctionInstructionGroup(address, 9)
  {
    if(f53)
      functions |= 0x01;
    if(f54)
      functions |= 0x02;
    if(f55)
      functions |= 0x04;
    if(f56)
      functions |= 0x08;
    if(f57)
      functions |= 0x10;
    if(f58)
      functions |= 0x20;
    if(f59)
      functions |= 0x40;
    if(f60)
      functions |= 0x80;

    checksum = calcChecksum(*this);
  }
} ATTRIBUTE_PACKED;

struct FunctionInstructionGroup10 : FunctionInstructionGroup
{
  FunctionInstructionGroup10(uint16_t address, bool f61, bool f62, bool f63, bool f64, bool f65, bool f66, bool f67, bool f68) :
    FunctionInstructionGroup(address, 10)
  {
    if(f61)
      functions |= 0x01;
    if(f62)
      functions |= 0x02;
    if(f63)
      functions |= 0x04;
    if(f64)
      functions |= 0x08;
    if(f65)
      functions |= 0x10;
    if(f66)
      functions |= 0x20;
    if(f67)
      functions |= 0x40;
    if(f68)
      functions |= 0x80;

    checksum = calcChecksum(*this);
  }
} ATTRIBUTE_PACKED;

/*
struct setFunctionStateGroup : LocomotiveInstruction
{
  uint8_t state = 0x00;
  uint8_t checksum;

  setFunctionStateGroup(uint8_t group, uint16_t address)
  {
    assert(group >= 1 && group <= 3);
    header = SET_LOCO;
    identification = 0x23 + group;
    addressLowHigh(address, addressLow, addressHigh);
  }
} ATTRIBUTE_PACKED;
*/

struct LocomotiveInfoBase : Message
{
  static constexpr uint8_t identificationMask = 0xF0;
  static constexpr uint8_t db2_busy_flag = 0x08;
  static constexpr uint8_t db2_speed_steps_14 = 0x00;
  static constexpr uint8_t db2_speed_steps_27 = 0x01;
  static constexpr uint8_t db2_speed_steps_28 = 0x02;
  static constexpr uint8_t db2_speed_steps_128 = 0x04;
  static constexpr uint8_t db2_speed_steps_mask = 0x07;
  static constexpr uint8_t directionFlag = 0x80;
  static constexpr uint8_t speedStepMask = 0x7F;
  static constexpr uint8_t flagF0 = 0x10;

  uint8_t identification;
  uint8_t speedAndDirection = 0x00;
  uint8_t functions1 = 0x00;
  uint8_t functions2 = 0x00;

  LocomotiveInfoBase(uint8_t _header) :
    Message(_header)
  {
  }

  inline bool isBusy() const
  {
    return identification & db2_busy_flag;
  }

  inline void setBusy(bool value)
  {
    if(value)
      identification |= db2_busy_flag;
    else
      identification &= ~db2_busy_flag;
  }

  inline uint8_t speedSteps() const
  {
    switch(identification & db2_speed_steps_mask)
    {
    case db2_speed_steps_14:  return 14;
    case db2_speed_steps_27:  return 27;
    case db2_speed_steps_28:  return 28;
    case db2_speed_steps_128: return 126;
    }
    return 0;
  }

  inline void setSpeedSteps(uint8_t value)
  {
    identification &= ~db2_speed_steps_mask;
    switch(value)
    {
    case 14:  identification |= db2_speed_steps_14;  break;
    case 27:  identification |= db2_speed_steps_27;  break;
    case 28:  identification |= db2_speed_steps_28;  break;
    case 126:
    case 128:
    default:  identification |= db2_speed_steps_128; break;
    }
  }

  inline Direction direction() const
  {
    return Z21::Utils::getDirection(speedAndDirection);
  }

  inline void setDirection(Direction value)
  {
    Z21::Utils::setDirection(speedAndDirection, value);
  }

  inline bool isEmergencyStop() const
  {
    return Z21::Utils::isEmergencyStop(speedAndDirection, speedSteps());
  }

  inline void setEmergencyStop()
  {
    Z21::Utils::setEmergencyStop(speedAndDirection);
  }

  inline uint8_t speedStep() const
  {
    return Z21::Utils::getSpeedStep(speedAndDirection, speedSteps());
  }

  inline void setSpeedStep(uint8_t value)
  {
    Z21::Utils::setSpeedStep(speedAndDirection, speedSteps(), value);
  }

protected:
  inline bool getFunctionBase(uint8_t index) const
  {
    assert(index >= 0 && index <= 12);
    if(index == 0)
      return functions1 & flagF0;
    else if(index <= 4)
      return functions1 & (1 << (index - 1));
    else if(index <= 12)
      return functions2 & (1 << (index - 5));
    else
      return false;
  }

  inline void setFunctionBase(uint8_t index, bool value)
  {
    assert(index >= 0 && index <= 12);
    if(index == 0)
    {
      if(value)
        functions1 |= flagF0;
      else
        functions1 &= ~flagF0;
    }
    else if(index <= 4)
    {
      const uint8_t flag = (1 << (index - 1));
      if(value)
        functions1 |= flag;
      else
        functions1 &= ~flag;
    }
    else if(index <= 12)
    {
      const uint8_t flag = (1 << (index - 5));
      if(value)
        functions2 |= flag;
      else
        functions2 &= ~flag;
    }
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LocomotiveInfoBase) == 5);

// 2.19.1 Locomotive information reply (from Central version 3.0)
struct LocomotiveInfo : LocomotiveInfoBase
{
  uint8_t checksum = 0x00;

  LocomotiveInfo() :
    LocomotiveInfoBase(SET_LOCO)
  {
  }

  inline bool getFunction(uint8_t index) const
  {
    return getFunctionBase(index);
  }

  inline void setFunction(uint8_t index, bool value)
  {
    return setFunctionBase(index, value);
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LocomotiveInfo) == 6);

// 2.19.2 Function status reply F13 to F28  (from Central version 3.6)
struct FunctionInfoF13F28 : Message
{
  uint8_t identification;
  uint8_t functions1 = 0;
  uint8_t functions2 = 0;
  uint8_t checksum = 0;

  FunctionInfoF13F28() :
    Message(GET_LOCO_INFO)
  {
    identification = idReplyFuncF13F28;
  }

  FunctionInfoF13F28(bool f13, bool f14, bool f15, bool f16, bool f17, bool f18, bool f19, bool f20,
                            bool f21, bool f22, bool f23, bool f24, bool f25, bool f26, bool f27, bool f28) :
    FunctionInfoF13F28()
  {
    if(f13)
      functions1 |= 0x01;
    if(f14)
      functions1 |= 0x02;
    if(f15)
      functions1 |= 0x04;
    if(f16)
      functions1 |= 0x08;
    if(f17)
      functions1 |= 0x10;
    if(f18)
      functions1 |= 0x20;
    if(f19)
      functions1 |= 0x40;
    if(f20)
      functions1 |= 0x80;

    if(f21)
      functions2 |= 0x01;
    if(f22)
      functions2 |= 0x02;
    if(f23)
      functions2 |= 0x04;
    if(f24)
      functions2 |= 0x08;
    if(f25)
      functions2 |= 0x10;
    if(f26)
      functions2 |= 0x20;
    if(f27)
      functions2 |= 0x40;
    if(f28)
      functions2 |= 0x80;

    checksum = calcChecksum(*this);
  }

  bool getFunction(uint8_t index) const
  {
    assert(index >= 13 && index <= 28);
    if(index <= 20)
      return functions1 & (1 << (index - 13));
    else
      return functions2 & (1 << (index - 21));
  }

  void setFunction(uint8_t index, bool value)
  {
    assert(index >= 13 && index <= 28);
    if(index <= 20)
    {
      const uint8_t flag = (1 << (index - 13));
      if(value)
        functions1 |= flag;
      else
        functions1 &= ~flag;
    }
    else
    {
      const uint8_t flag = (1 << (index - 21));
      if(value)
        functions2 |= flag;
      else
        functions2 &= ~flag;
    }
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(FunctionInfoF13F28) == 5);

// 2.19.3 Function status reply F29 to F68  (from Central version 4.0)
struct FunctionInfoF29F68 : Message
{
  uint8_t identification;
  uint8_t functions1 = 0;
  uint8_t functions2 = 0;
  uint8_t functions3 = 0;
  uint8_t functions4 = 0;
  uint8_t functions5 = 0;
  uint8_t checksum = 0;

  FunctionInfoF29F68() :
    Message(FUNC_INFO_V4)
  {
    identification = idReplyFuncF29F68;
  }

  bool getFunction(uint8_t index) const
  {
    assert(index >= 29 && index <= 68);
    if(index <= 36)
      return functions1 & (1 << (index - 29));
    else if(index <= 44)
      return functions2 & (1 << (index - 37));
    else if(index <= 52)
      return functions3 & (1 << (index - 45));
    else if(index <= 60)
      return functions4 & (1 << (index - 53));
    else
      return functions5 & (1 << (index - 61));
  }

  void setFunction(uint8_t index, bool value)
  {
    assert(index >= 29 && index <= 68);
    if(index <= 36)
    {
      const uint8_t flag = (1 << (index - 29));
      if(value)
        functions1 |= flag;
      else
        functions1 &= ~flag;
    }
    else if(index <= 44)
    {
      const uint8_t flag = (1 << (index - 37));
      if(value)
        functions2 |= flag;
      else
        functions2 &= ~flag;
    }
    else if(index <= 52)
    {
      const uint8_t flag = (1 << (index - 45));
      if(value)
        functions3 |= flag;
      else
        functions3 &= ~flag;
    }
    else if(index <= 60)
    {
      const uint8_t flag = (1 << (index - 53));
      if(value)
        functions4 |= flag;
      else
        functions4 &= ~flag;
    }
    else
    {
      const uint8_t flag = (1 << (index - 61));
      if(value)
        functions5 |= flag;
      else
        functions5 &= ~flag;
    }
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(FunctionInfoF29F68) == 8);

// 2.19.7 Locomotive is Occupied (from Central version 3.0)
struct LocomotiveBusy : LocomotiveInstruction
{
  uint8_t checksum;

  LocomotiveBusy(uint16_t address)
    : LocomotiveInstruction(address)
  {
    header = GET_LOCO_INFO;
    identification = idLocomotiveBusy;
    checksum = calcChecksum(*this);
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LocomotiveBusy) == 5);

// 3.24 Software version request
struct QueryCentralVersion : Message
{
  uint8_t db1 = idCentralVersion;
  uint8_t checksum = 0x00;

  QueryCentralVersion()
  {
    header = STOP_REQUEST;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(CentralVersionReplyOLD) == 4);

// 3.40.3 Query Locomotive state (from Central version 3.0)
struct QueryLocomotiveV3 : LocomotiveInstruction
{
  uint8_t checksum;

  QueryLocomotiveV3(uint16_t address)
    : LocomotiveInstruction(address)
  {
    header = GET_LOCO_INFO;
    identification = 0;
    checksum = calcChecksum(*this);
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(QueryLocomotiveV3) == 5);

// 3.40.7 Query Locomotive functions (from Central version 3.6), and 3.40.8 (from Central version 4.0)
struct QueryLocomotiveFunctions : LocomotiveInstruction
{
  uint8_t checksum;

  QueryLocomotiveFunctions(uint16_t address, uint8_t group)
    : LocomotiveInstruction(address)
  {
    assert(group >= 4 && group <= 10);
    header = GET_LOCO_INFO;

    if(group == 4 || group == 5)
      identification = idQueryFuncGroup4and5;
    else
      identification = idQueryFuncGroup6above;
    checksum = calcChecksum(*this);
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(QueryLocomotiveFunctions) == 5);

// 3.38 Set accessory state (OLD! up to Central version 3.6)
struct AccessoryDecoderOperationRequestOLD : Message
{
  static constexpr uint8_t db2Port = 0x01;
  static constexpr uint8_t db2Activate = 0x08;

  uint8_t db1 = 0x00;
  uint8_t db2 = 0x80;
  uint8_t checksum;

  AccessoryDecoderOperationRequestOLD(uint16_t address_, bool port_, bool activate_)
    : Message(SET_ACCESSORY_OLD)
  {
    assert(address_ >= 1 && address_ <= 1024);
    address_--;
    db1 = static_cast<uint8_t>(address_ >> 2);
    db2 |= static_cast<uint8_t>(address_ & 0x03) << 1;
    if(port_)
    {
      db2 |= db2Port;
    }
    if(activate_)
    {
      db2 |= db2Activate;
    }
    checksum = calcChecksum(*this);
  }

  uint16_t address() const
  {
    return 1 + ((static_cast<uint16_t>(db1) << 2) | ((db2 >> 1) & 0x03));
  }

  bool port() const
  {
    return db2 & db2Port;
  }

  bool activate() const
  {
    return db2 & db2Activate;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(AccessoryDecoderOperationRequestOLD) == 4);

// 3.39 Set accessory state (from Central version 3.8)
struct AccessoryDecoderOperationRequest : Message
{
  static constexpr uint8_t db3Port = 0x01;
  static constexpr uint8_t db3Activate = 0x08;

  uint8_t addressHigh = 0x00;
  uint8_t addressLow = 0x00;
  uint8_t db3 = 0x80;
  uint8_t checksum;

  AccessoryDecoderOperationRequest(uint16_t address_, bool port_, bool activate_)
    : Message(SET_ACCESSORY)
  {
    assert(address_ >= 1 && address_ <= 2048);
    address_--;
    addressHigh = address_ >> 10;
    addressLow = (address_ >> 2) & 0xFF;
    db3 |= static_cast<uint8_t>(address_ & 0x03) << 1;
    if(port_)
    {
      db3 |= db3Port;
    }
    if(activate_)
    {
      db3 |= db3Activate;
    }
    checksum = calcChecksum(*this);
  }

  uint16_t address() const
  {
    return 1 + ((to16(addressLow, addressHigh & 0x3F) << 2) | ((db3 >> 1) & 0x03));
  }

  bool port() const
  {
    return db3 & db3Port;
  }

  bool activate() const
  {
    return db3 & db3Activate;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(AccessoryDecoderOperationRequest) == 5);

namespace RocoMultiMAUS
{
  struct FunctionInstructionF13F20 : LocomotiveInstruction
  {
    uint8_t functions = 0x00;
    uint8_t checksum;

    FunctionInstructionF13F20(uint16_t address, bool f13, bool f14, bool f15, bool f16, bool f17, bool f18, bool f19, bool f20) :
      LocomotiveInstruction(address)
    {
      identification = idSetFuncGroup4_Roco;

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

    bool getFunction(uint8_t index) const
    {
      assert(index >= 13 && index <= 20);
      return (functions >> (index - 13) & 0x01);
    }
  } ATTRIBUTE_PACKED;
  static_assert(sizeof(FunctionInstructionF13F20) == 6);

  // multiMAUS V1.02 Query Locomotive state up to F20 and speed, direction info
  struct QueryLocomotiveCumulative : LocomotiveInstruction
  {
    uint8_t checksum;

    QueryLocomotiveCumulative(uint16_t address)
      : LocomotiveInstruction(address)
    {
      header = GET_LOCO_INFO;
      identification = idQueryLocoCumulative_Roco;
      checksum = calcChecksum(*this);
    }
  } ATTRIBUTE_PACKED;
  static_assert(sizeof(QueryLocomotiveCumulative) == 5);

  // multiMAUS V1.02 Locomotive state reply up to F20 and speed, direction info
  struct LocomotiveCumulativeInfo : LocomotiveInfoBase
  {
    uint8_t unused0 = 0x00;
    uint8_t functions4 = 0x00;
    uint8_t unused1 = 0x00;
    uint8_t checksum = 0x00;

    LocomotiveCumulativeInfo() :
      LocomotiveInfoBase(LOCO_INFO_CUMULATIVE)
    {

    }

    inline bool getFunction(uint8_t index) const
    {
      assert(index >= 0 && index <= 20);
      if(index <= 12)
        return getFunctionBase(index);
      else if(index <= 20)
        return functions4 & (1 << (index - 13));
      else
        return false;
    }

    inline void setFunction(uint8_t index, bool value)
    {
      assert(index >= 0 && index <= 20);
      if(index <= 12)
      {
        setFunctionBase(index, value);
      }
      else if(index <= 20)
      {
        const uint8_t flag = (1 << (index - 13));
        if(value)
          functions4 |= flag;
        else
          functions4 &= ~flag;
      }
    }
  } ATTRIBUTE_PACKED;
  static_assert(sizeof(LocomotiveCumulativeInfo) == 9);
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
  } ATTRIBUTE_PACKED;
  static_assert(sizeof(S88StartAddress) == 4);

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
  } ATTRIBUTE_PACKED;
  static_assert(sizeof(S88ModuleCount) == 4);
}

inline uint8_t calcChecksum(const Message& msg)
{
  return calcChecksum(msg, msg.dataSize());
}

inline bool isChecksumValid(const Message& msg)
{
  return isChecksumValid(msg, msg.dataSize());
}

}

inline bool operator ==(const XpressNet::Message& lhs, const XpressNet::Message& rhs)
{
  return lhs.size() == rhs.size() && std::memcmp(&lhs, &rhs, lhs.size()) == 0;
}

#endif
