/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DINAMO_DINAMOMESSAGES_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DINAMO_DINAMOMESSAGES_HPP

#include <array>
#include <cassert>
#include <cstdint>
#include <optional>
#include <string>
#include <span>
#include <traintastic/enum/direction.hpp>
#include "dinamopolarity.hpp"
#include "dinamotypes.hpp"
#include "../../../utils/bit.hpp"

namespace Dinamo {

enum class Command : uint8_t
{
  SubSystem = 0x00,
  SystemControl = 0x01,
};

enum class SubCommand : uint8_t
{
  ResetFault = 0x00,
  SetHFILevel = 0x01,
  ProtocolVersion = 0x02,
  SystemVersion = 0x0A,
};

struct Message
{
};

struct SubSystemCommand : Message
{
  enum class Type : uint8_t
  {
    TM44_TMH_TMCC = 0x20,
    PM32 = 0x40,
    OM32_OC32 = 0x60,
    OC32XA = 0x70,
  };

  static constexpr uint8_t addressMask(Type type)
  {
    switch(type)
    {
      case Type::TM44_TMH_TMCC:
        return 0x1F;

      case Type::PM32:
      case Type::OM32_OC32:
      case Type::OC32XA:
        return 0x0F;
    }
    return 0x00;
  }

  Command command = Command::SubSystem;
  uint8_t id; // [T T T/A A A A A]

  SubSystemCommand(Type type_, uint8_t address_)
    : id(static_cast<uint8_t>(type_) | (address_ & addressMask(type_)))
  {
    assert(address_ <= addressMask(type_));
  }

  Type type() const
  {
    if(getBit<6>(id)) // 3 bit type
    {
      return static_cast<Type>(id & 0x70);
    }
    else // 2 bit type
    {
      return static_cast<Type>(id & 0x60);
    }
  }

  uint8_t address() const
  {
    return (id & addressMask(type()));
  }
};

/*
struct SubSystemIdentify : SubSystemCommand
{
  uint8_t systemCommand = 0x00;

  SubSystemIdentify(Type type_, uint8_t address_)
    : SubSystemCommand(type_, address_)
  {
  }
};
*/

struct SystemCommand : Message
{
  Command command = Command::SystemControl;
  SubCommand subCommand;

  SystemCommand(SubCommand subCmd)
    : subCommand{subCmd}
  {
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() >= sizeof(SystemCommand)) &&
      (message[0] == static_cast<uint8_t>(Command::SystemControl));
  }
};
static_assert(sizeof(SystemCommand) == 2);

struct ResetFault : SystemCommand
{
  ResetFault()
    : SystemCommand(SubCommand::ResetFault)
  {
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(ResetFault)) &&
      SystemCommand::check(message) &&
      reinterpret_cast<const SystemCommand*>(message.data())->subCommand == SubCommand::ResetFault;
  }
};
static_assert(sizeof(ResetFault) == 2);

struct SetHFILevel : SystemCommand
{
  uint8_t level;

  SetHFILevel(uint8_t value)
    : SystemCommand(SubCommand::SetHFILevel)
    , level(value & 0x0F)
  {
    assert(value <= 0x0F);
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(SetHFILevel)) &&
      SystemCommand::check(message) &&
      reinterpret_cast<const SystemCommand*>(message.data())->subCommand == SubCommand::SetHFILevel;
  }
};
static_assert(sizeof(SetHFILevel) == 3);

struct ProtocolVersionRequest : SystemCommand
{
  ProtocolVersionRequest()
    : SystemCommand(SubCommand::ProtocolVersion)
  {
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(ProtocolVersionRequest)) &&
      SystemCommand::check(message) &&
      reinterpret_cast<const SystemCommand*>(message.data())->subCommand == SubCommand::ProtocolVersion;
  }
};
static_assert(sizeof(ProtocolVersionRequest) == 2);

struct ProtocolVersionResponse : SystemCommand
{
  uint8_t majorMinor;
  uint8_t subReleaseBugFix;

  ProtocolVersionResponse(uint8_t major_, uint8_t minor_, uint8_t subRelease_, uint8_t bugFix_)
    : SystemCommand(SubCommand::ProtocolVersion)
    , majorMinor(((major_ & 0x07) << 3) | (minor_ & 0x07))
    , subReleaseBugFix(((subRelease_ & 0x07) << 3) | (bugFix_ & 0x07))
  {
    assert(major_ <= 0x07);
    assert(minor_ <= 0x07);
    assert(subRelease_ <= 0x07);
    assert(bugFix_ <= 0x07);
  }

  uint8_t major() const
  {
    return ((majorMinor >> 3) & 0x07);
  }

  uint8_t minor() const
  {
    return (majorMinor & 0x07);
  }

  uint8_t subRelease() const
  {
    return ((subReleaseBugFix >> 3) & 0x07);
  }

  uint8_t bugFix() const
  {
    return (subReleaseBugFix & 0x07);
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(ProtocolVersionResponse)) &&
      SystemCommand::check(message) &&
      reinterpret_cast<const SystemCommand*>(message.data())->subCommand == SubCommand::ProtocolVersion;
  }
};
static_assert(sizeof(ProtocolVersionResponse) == 4);

struct SystemVersionRequest : SystemCommand
{
  SystemVersionRequest()
    : SystemCommand(SubCommand::SystemVersion)
  {
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(SystemVersionRequest)) &&
      SystemCommand::check(message) &&
      reinterpret_cast<const SystemCommand*>(message.data())->subCommand == SubCommand::SystemVersion;
  }
};
static_assert(sizeof(SystemVersionRequest) == 2);

struct SystemVersionResponse : SystemCommand
{
  SystemType type;
  uint8_t majorMinor;
  uint8_t subReleaseBugFix;

  SystemVersionResponse(SystemType type_, uint8_t major_, uint8_t minor_, uint8_t subRelease_, uint8_t bugFix_)
    : SystemCommand(SubCommand::SystemVersion)
    , type{type_}
    , majorMinor(((major_ & 0x07) << 3) | (minor_ & 0x07))
    , subReleaseBugFix(((subRelease_ & 0x07) << 3) | (bugFix_ & 0x07))
  {
    assert(major_ <= 0x07);
    assert(minor_ <= 0x07);
    assert(subRelease_ <= 0x07);
    assert(bugFix_ <= 0x07);
  }

  uint8_t major() const
  {
    return ((majorMinor >> 3) & 0x07);
  }

  uint8_t minor() const
  {
    return (majorMinor & 0x07);
  }

  uint8_t subRelease() const
  {
    return ((subReleaseBugFix >> 3) & 0x07);
  }

  uint8_t bugFix() const
  {
    return (subReleaseBugFix & 0x07);
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(SystemVersionResponse)) &&
      SystemCommand::check(message) &&
      reinterpret_cast<const SystemCommand*>(message.data())->subCommand == SubCommand::SystemVersion;
  }
};
static_assert(sizeof(SystemVersionResponse) == 5);

struct SetOutput : Message
{
  static constexpr uint8_t headerId = 0x10;

  uint8_t data1; // [0  1  0  0  C  A8 A7]
  uint8_t data2; // [A6 A5 A4 A3 A2 A1 A0]

  SetOutput(uint16_t addr, bool coil)
    : data1(headerId | (coil ? 0x04 : 0x00) | ((addr >> 7) & 0x03))
    , data2(addr & 0x3F)
  {
    assert(addr < 512);
  }

  uint16_t address() const
  {
    return ((static_cast<uint16_t>(data1) & 0x03) << 7) | (data2 & 0x7F);
  }

  bool coil() const
  {
    return getBit<2>(data1);
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(SetOutput)) &&
      ((message[0] & 0x78) == headerId);
  }
};
static_assert(sizeof(SetOutput) == 2);

struct SetOutputWithPulseDuration : SetOutput
{
  uint8_t duration;   // 0..127 in 1/60s steps

  SetOutputWithPulseDuration(uint16_t addr, bool coil, uint8_t duration_)
    : SetOutput(addr, coil)
    , duration(duration_ & 0x7F)
  {
    assert(duration_ <= 0x7F);
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(SetOutputWithPulseDuration)) &&
      ((message[0] & 0x78) == headerId);
  }
};
static_assert(sizeof(SetOutputWithPulseDuration) == 3);

struct Ox32 : Message
{
  enum class Command : uint8_t
  {
    SetAspect = 1,
  };

  static constexpr uint8_t headerId = 0x18;

  uint8_t data1; // [0  0  1  1  M5 M4 M3]
  uint8_t data2; // [M1 M0 O4 O3 O2 O1 O0]
  Command command;
  uint8_t parameter;

  Ox32(uint8_t module_, uint8_t output_, Command command_, uint8_t parameter_)
    : data1(headerId | ((module_ >> 2) & 0x07))
    , data2(((module_ & 0x03) << 5) | (output_ & 0x1F))
    , command{command_}
    , parameter{parameter_}
  {
    assert(module_ <= 0x1F);
    assert(output_ <= 0x1F);
    assert(static_cast<uint8_t>(command_) <= 0x7F);
    assert(parameter_ <= 0x7F);
  }

  uint8_t module() const
  {
    return ((data1 << 3) & 0x18) | ((data2 >> 5) & 0x03);
  }

  uint8_t output() const
  {
    return (data2 & 0x1F);
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(Ox32)) &&
      ((message[0] & 0x78) == headerId);
  }
};
static_assert(sizeof(Ox32) == 4);

struct BlockMessage : Message
{
  static constexpr uint8_t headerIdMask = 0x7E;

  uint8_t data1; // [H5 H4 H3 H3 H1 H0 B7]
  uint8_t data2; // [B6 B5 B4 B3 B2 B1 B0]

  BlockMessage(uint8_t headerId, uint8_t block_)
    : data1((headerId & headerIdMask) | (block_ >> 7))
    , data2(block_ & 0x7F)
  {
  }

  uint8_t block() const
  {
    return (data1 << 7) | data2;
  }
};

struct BlockAnalogSetSpeed : BlockMessage
{
  static constexpr uint8_t headerId = 0x20;
  static constexpr uint8_t speedMask = 0x3F;

  uint8_t data3; // [1 S5 S4 S3 S2 S1 S0]

  BlockAnalogSetSpeed(uint8_t block_, uint8_t speed_)
    : BlockMessage(headerId, block_)
    , data3(0x40 | (speed_ & speedMask))
  {
    assert(speed_ <= speedMask);
  }

  uint8_t speed() const
  {
    return (data3 & speedMask);
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(BlockAnalogSetSpeed)) &&
      ((message[0] & headerIdMask) == headerId) &&
      ((message[2] & 0x40) == 0x40);
  }
};
static_assert(sizeof(BlockAnalogSetSpeed) == 3);

struct BlockAnalogSetSpeedPolarity : BlockMessage
{
  static constexpr uint8_t headerId = 0x22;
  static constexpr uint8_t speedMask = 0x3F;
  static constexpr uint8_t polarityBit = 6;
  static constexpr uint8_t polarityMask = 1 << polarityBit;

  uint8_t data3; // [P S5 S4 S3 S2 S1 S0]

  BlockAnalogSetSpeedPolarity(uint8_t block_, uint8_t speed_, Polarity polarity_)
    : BlockMessage(0x22, block_)
    , data3((polarity_ == Polarity::Positive ? 0x40 : 0x00) | (speed_ & speedMask))
  {
    assert(speed_ <= speedMask);
    assert(polarity_ == Polarity::Positive || polarity_ == Polarity::Negative);
  }

  uint8_t speed() const
  {
    return (data3 & speedMask);
  }

  Polarity polarity() const
  {
    return getBit<6>(data3) ? Polarity::Positive : Polarity::Negative;
  }

  static bool check(std::span<const uint8_t> message)
  {
    return (message.size() == sizeof(BlockAnalogSetSpeedPolarity)) && ((message[0] & 0x7E) == 0x22);
  }
};
static_assert(sizeof(BlockAnalogSetSpeedPolarity) == 3);

struct BlockAnalogSetLight : BlockMessage
{
  static constexpr uint8_t headerId = 0x20;
  static constexpr uint8_t lightBit = 6;
  static constexpr uint8_t lightMask = 1 << lightBit;

  uint8_t data3; // [0 0 L 0 0 0 0]

  BlockAnalogSetLight(uint8_t block_, bool on)
    : BlockMessage(headerId, block_)
    , data3(on ? lightMask : 0x00)
  {
  }

  bool light() const
  {
    return getBit<lightBit>(data3);
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(BlockAnalogSetLight)) &&
      ((message[0] & headerIdMask) == headerId) &&
      ((message[2] & 0x6F) == 0x00);
  }
};
static_assert(sizeof(BlockAnalogSetLight) == 3);

struct DCCShortAddress
{
  uint8_t address1; // [A6 A5 A4 A3 A2 A1 A0]

  DCCShortAddress(uint8_t address_)
    : address1(address_ & 0x7F)
  {
    assert(address_ <= 0x7F);
  }

  uint8_t address() const
  {
    return (address1 & 0x7F);
  }
};

struct DCCLongAddress
{
  uint8_t address1; // [A6  A5  A4  A3  A2 A1 A0]
  uint8_t address2; // [A13 A12 A11 A10 A9 A8 A7]

  DCCLongAddress(uint16_t address_)
    : address1(address_ & 0x7F)
    , address2((address_ >> 7) & 0x7F)
  {
    assert(address_ <= 10239);
  }

  uint16_t address() const
  {
    return (static_cast<uint16_t>(address2) << 7) | (address1 & 0x7F);
  }
};

struct BlockDCCSpeedDirection : BlockMessage
{
  static constexpr uint8_t headerId = 0x28;
  static constexpr uint8_t speedMask = 0x1F;
  static constexpr uint8_t speedEStop = 0x1F;
  static constexpr uint8_t directionBit = 5;
  static constexpr uint8_t directionMask = 1 << directionBit;

  uint8_t data3; // [1 D S4 S3 S2 S1 S0]

  BlockDCCSpeedDirection(uint8_t block_, bool emergencyStop_, uint8_t speed_, Direction direction_)
    : BlockMessage(headerId, block_)
    , data3(0x40 | (direction_ == Direction::Forward ? directionMask : 0x00) | (emergencyStop_ ? speedEStop : (speed_ & speedMask)))
  {
    assert(speed_ <= 28);
    assert(direction_ == Direction::Forward || direction_ == Direction::Reverse);
  }

  bool emergencyStop() const
  {
    return (data3 & speedMask) == speedEStop;
  }

  uint8_t speed() const
  {
    return !emergencyStop() ? (data3 & speedMask) : 0;
  }

  Direction direction() const
  {
    return getBit<directionBit>(data3) ? Direction::Forward : Direction::Reverse;
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() >= sizeof(BlockDCCSpeedDirection)) &&
      ((message[0] & headerIdMask) == headerId) &&
      ((message[2] & 0x40) == 0x40);
  }
};
static_assert(sizeof(BlockDCCSpeedDirection) == 3);

struct BlockDCCShortSpeedDirection : BlockDCCSpeedDirection, DCCShortAddress
{
  BlockDCCShortSpeedDirection(uint8_t block_, uint8_t address_, bool emergencyStop_, uint8_t speed_, Direction direction_)
    : BlockDCCSpeedDirection(block_, emergencyStop_, speed_, direction_)
    , DCCShortAddress(address_)
  {
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(BlockDCCShortSpeedDirection)) &&
      BlockDCCSpeedDirection::check(message);
  }
};
static_assert(sizeof(BlockDCCShortSpeedDirection) == 4);

struct BlockDCCLongSpeedDirection : BlockDCCSpeedDirection, DCCLongAddress
{
  BlockDCCLongSpeedDirection(uint8_t block_, uint16_t address_, bool emergencyStop_, uint8_t speed_, Direction direction_)
    : BlockDCCSpeedDirection(block_, emergencyStop_, speed_, direction_)
    , DCCLongAddress(address_)
  {
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(BlockDCCLongSpeedDirection)) &&
      BlockDCCSpeedDirection::check(message);
  }
};
static_assert(sizeof(BlockDCCLongSpeedDirection) == 5);

struct BlockDCCSpeedDirectionPolarity : BlockMessage
{
  static constexpr uint8_t headerId = 0x2A;
  static constexpr uint8_t speedMask = 0x1F;
  static constexpr uint8_t speedEStop = 0x1F;
  static constexpr uint8_t directionBit = 5;
  static constexpr uint8_t directionMask = 1 << directionBit;
  static constexpr uint8_t polarityBit = 5;
  static constexpr uint8_t polarityMask = 1 << polarityBit;

  uint8_t data3; // [P D S4 S3 S2 S1 S0]

  BlockDCCSpeedDirectionPolarity(uint8_t block_, bool emergencyStop_, uint8_t speed_, Direction direction_, Polarity polarity_)
    : BlockMessage(headerId, block_)
    , data3((polarity_ == Polarity::Positive ? polarityMask : 0x00) | (direction_ == Direction::Forward ? directionMask : 0x00) | (emergencyStop_ ? speedEStop : (speed_ & speedMask)))
  {
    assert(speed_ <= 28);
    assert(direction_ == Direction::Forward || direction_ == Direction::Reverse);
  }

  bool emergencyStop() const
  {
    return (data3 & speedMask) == speedEStop;
  }

  uint8_t speed() const
  {
    return !emergencyStop() ? (data3 & speedMask) : 0;
  }

  Direction direction() const
  {
    return getBit<directionBit>(data3) ? Direction::Forward : Direction::Reverse;
  }

  Polarity polarity() const
  {
    return getBit<polarityBit>(data3) ? Polarity::Positive : Polarity::Negative;
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() >= sizeof(BlockDCCSpeedDirectionPolarity)) &&
      ((message[0] & headerIdMask) == headerId);
  }
};
static_assert(sizeof(BlockDCCSpeedDirectionPolarity) == 3);

struct BlockDCCShortSpeedDirectionPolarity : BlockDCCSpeedDirectionPolarity, DCCShortAddress
{
  BlockDCCShortSpeedDirectionPolarity(uint8_t block_, uint8_t address_, bool emergencyStop_, uint8_t speed_, Direction direction_, Polarity polarity_)
    : BlockDCCSpeedDirectionPolarity(block_, emergencyStop_, speed_, direction_, polarity_)
    , DCCShortAddress(address_)
  {
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(BlockDCCShortSpeedDirectionPolarity)) &&
      BlockDCCSpeedDirection::check(message);
  }
};
static_assert(sizeof(BlockDCCShortSpeedDirectionPolarity) == 4);

struct BlockDCCLongSpeedDirectionPolarity : BlockDCCSpeedDirectionPolarity, DCCLongAddress
{
  BlockDCCLongSpeedDirectionPolarity(uint8_t block_, uint16_t address_, bool emergencyStop_, uint8_t speed_, Direction direction_, Polarity polarity_)
    : BlockDCCSpeedDirectionPolarity(block_, emergencyStop_, speed_, direction_, polarity_)
    , DCCLongAddress(address_)
  {
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(BlockDCCLongSpeedDirectionPolarity)) &&
      BlockDCCSpeedDirectionPolarity::check(message);
  }
};
static_assert(sizeof(BlockDCCLongSpeedDirectionPolarity) == 5);

struct BlockDCCFunctionF0F4 : BlockMessage
{
  static constexpr uint8_t headerId = 0x28;
  static constexpr uint8_t f0Bit = 4;
  static constexpr uint8_t f1Bit = 0;
  static constexpr uint8_t f2Bit = 1;
  static constexpr uint8_t f3Bit = 2;
  static constexpr uint8_t f4Bit = 3;

  uint8_t data3; // [0 0 F0 F4 F3 F2 F1]

  bool f0() const
  {
    return getBit<f0Bit>(data3);
  }

  bool f1() const
  {
    return getBit<f1Bit>(data3);
  }

  bool f2() const
  {
    return getBit<f2Bit>(data3);
  }

  bool f3() const
  {
    return getBit<f3Bit>(data3);
  }

  bool f4() const
  {
    return getBit<f4Bit>(data3);
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() >= sizeof(BlockDCCFunctionF0F4)) &&
      ((message[0] & headerIdMask) == headerId) &&
      ((message[2] & 0x60) == 0x00);
  }

protected:
  BlockDCCFunctionF0F4(uint8_t block_, bool f0, bool f1, bool f2, bool f3, bool f4)
    : BlockMessage(headerId, block_)
    , data3{0x00}
  {
    setBit<f0Bit>(data3, f0);
    setBit<f1Bit>(data3, f1);
    setBit<f2Bit>(data3, f2);
    setBit<f3Bit>(data3, f3);
    setBit<f4Bit>(data3, f4);
  }
};

struct BlockDCCShortFunctionF0F4 : BlockDCCFunctionF0F4, DCCShortAddress
{
  BlockDCCShortFunctionF0F4(uint8_t block_, uint8_t address_, bool f0, bool f1, bool f2, bool f3, bool f4)
    : BlockDCCFunctionF0F4(block_, f0, f1, f2, f3, f4)
    , DCCShortAddress(address_)
  {
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(BlockDCCShortFunctionF0F4)) &&
      BlockDCCFunctionF0F4::check(message);
  }
};
static_assert(sizeof(BlockDCCShortFunctionF0F4) == 4);

struct BlockDCCLongFunctionF0F4 : BlockDCCFunctionF0F4, DCCLongAddress
{
  BlockDCCLongFunctionF0F4(uint8_t block_, uint16_t address_, bool f0, bool f1, bool f2, bool f3, bool f4)
    : BlockDCCFunctionF0F4(block_, f0, f1, f2, f3, f4)
    , DCCLongAddress(address_)
  {
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(BlockDCCLongFunctionF0F4)) &&
      BlockDCCFunctionF0F4::check(message);
  }
};
static_assert(sizeof(BlockDCCLongFunctionF0F4) == 5);

struct BlockDCCFunctionF5F8 : BlockMessage
{
  static constexpr uint8_t headerId = 0x28;
  static constexpr uint8_t f5Bit = 0;
  static constexpr uint8_t f6Bit = 1;
  static constexpr uint8_t f7Bit = 2;
  static constexpr uint8_t f8Bit = 3;

  uint8_t data3; // [0 1 0 F8 F7 F6 F5]

  bool f5() const
  {
    return getBit<f5Bit>(data3);
  }

  bool f6() const
  {
    return getBit<f6Bit>(data3);
  }

  bool f7() const
  {
    return getBit<f7Bit>(data3);
  }

  bool f8() const
  {
    return getBit<f8Bit>(data3);
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() >= sizeof(BlockDCCFunctionF5F8)) &&
      ((message[0] & headerIdMask) == headerId) &&
      ((message[2] & 0x70) == 0x20);
  }

protected:
  BlockDCCFunctionF5F8(uint8_t block_, bool f5, bool f6, bool f7, bool f8)
    : BlockMessage(headerId, block_)
    , data3{0x20}
  {
    setBit<f5Bit>(data3, f5);
    setBit<f6Bit>(data3, f6);
    setBit<f7Bit>(data3, f7);
    setBit<f8Bit>(data3, f8);
  }
};

struct BlockDCCShortFunctionF5F8 : BlockDCCFunctionF5F8, DCCShortAddress
{
  BlockDCCShortFunctionF5F8(uint8_t block_, uint8_t address_, bool f5, bool f6, bool f7, bool f8)
    : BlockDCCFunctionF5F8(block_, f5, f6, f7, f8)
    , DCCShortAddress(address_)
  {
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(BlockDCCShortFunctionF5F8)) &&
      BlockDCCFunctionF5F8::check(message);
  }
};
static_assert(sizeof(BlockDCCShortFunctionF5F8) == 4);

struct BlockDCCLongFunctionF5F8 : BlockDCCFunctionF5F8, DCCLongAddress
{
  BlockDCCLongFunctionF5F8(uint8_t block_, uint16_t address_, bool f5, bool f6, bool f7, bool f8)
    : BlockDCCFunctionF5F8(block_, f5, f6, f7, f8)
    , DCCLongAddress(address_)
  {
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(BlockDCCLongFunctionF5F8)) &&
      BlockDCCFunctionF5F8::check(message);
  }
};
static_assert(sizeof(BlockDCCLongFunctionF5F8) == 5);

struct BlockDCCFunctionF9F12 : BlockMessage
{
  static constexpr uint8_t headerId = 0x28;
  static constexpr uint8_t f9Bit = 0;
  static constexpr uint8_t f10Bit = 1;
  static constexpr uint8_t f11Bit = 2;
  static constexpr uint8_t f12Bit = 3;

  uint8_t data3; // [0 1 1 F9 F10 F11 F12]

  bool f9() const
  {
    return getBit<f9Bit>(data3);
  }

  bool f10() const
  {
    return getBit<f10Bit>(data3);
  }

  bool f11() const
  {
    return getBit<f11Bit>(data3);
  }

  bool f12() const
  {
    return getBit<f12Bit>(data3);
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() >= sizeof(BlockDCCFunctionF9F12)) &&
      ((message[0] & headerIdMask) == headerId) &&
      ((message[2] & 0x70) == 0x30);
  }

protected:
  BlockDCCFunctionF9F12(uint8_t block_, bool f9, bool f10, bool f11, bool f12)
    : BlockMessage(headerId, block_)
    , data3{0x30}
  {
    setBit<f9Bit>(data3, f9);
    setBit<f10Bit>(data3, f10);
    setBit<f11Bit>(data3, f11);
    setBit<f12Bit>(data3, f12);
  }
};

struct BlockDCCShortFunctionF9F12 : BlockDCCFunctionF9F12, DCCShortAddress
{
  BlockDCCShortFunctionF9F12(uint8_t block_, uint8_t address_, bool f9, bool f10, bool f11, bool f12)
    : BlockDCCFunctionF9F12(block_, f9, f10, f11, f12)
    , DCCShortAddress(address_)
  {
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(BlockDCCShortFunctionF9F12)) &&
      BlockDCCFunctionF9F12::check(message);
  }
};
static_assert(sizeof(BlockDCCShortFunctionF9F12) == 4);

struct BlockDCCLongFunctionF9F12 : BlockDCCFunctionF9F12, DCCLongAddress
{
  BlockDCCLongFunctionF9F12(uint8_t block_, uint16_t address_, bool f9, bool f10, bool f11, bool f12)
    : BlockDCCFunctionF9F12(block_, f9, f10, f11, f12)
    , DCCLongAddress(address_)
  {
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(BlockDCCLongFunctionF9F12)) &&
      BlockDCCFunctionF9F12::check(message);
  }
};
static_assert(sizeof(BlockDCCLongFunctionF9F12) == 5);

struct BlockAlarm : BlockMessage
{
  static constexpr uint8_t headerId = 0x30;
  static constexpr uint8_t shortCircuitBit = 0;

  BlockAlarm(uint8_t block_, bool shortCircuit_ = false)
    : BlockMessage(headerId, block_)
  {
    setBit<shortCircuitBit>(data1, shortCircuit_);
  }

  bool shortCircuit() const
  {
    return getBit<shortCircuitBit>(data1);
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(BlockAlarm)) &&
      ((message[0] & 0x7C) == headerId);
  }
};
static_assert(sizeof(BlockAlarm) == 2);

struct BlockLink : BlockMessage
{
  static constexpr uint8_t headerId = 0x3A;
  static constexpr uint8_t permanentBit = 1; // 0 = copy, 1 = link
  static constexpr uint8_t invertPolarityBit = 2;

  uint8_t data3; // [0  0  0  0  I  P  S7]
  uint8_t data4; // [S6 S5 S4 S3 S2 S1 S0]

  BlockLink(uint8_t block_, uint8_t sourceBlock_, bool invertPolarity_, bool link = true)
    : BlockMessage(headerId, block_)
    , data3(sourceBlock_ >> 7)
    , data4(sourceBlock_ & 0x7F)
  {
    setBit<permanentBit>(data3, link);
    setBit<invertPolarityBit>(data3, invertPolarity_);
  }

  uint8_t sourceBlock() const
  {
    return (data3 << 7) | (data4 & 0x7F);
  }

  bool invertPolarity() const
  {
    return getBit<invertPolarityBit>(data3);
  }

  bool link() const
  {
    return getBit<permanentBit>(data3);
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(BlockLink)) &&
      ((message[0] & headerIdMask) == headerId) &&
      ((message[2] & 0x78) == 0x00);
  }
};
static_assert(sizeof(BlockLink) == 4);

struct BlockUnlink : BlockMessage
{
  static constexpr uint8_t headerId = 0x38;
  static constexpr uint8_t zeroBit = 1;
  static constexpr uint8_t unlinkBit = 2; // 0 = down, 1 = up

  uint8_t data3; // [0 0 0 0 U Z 0]

  BlockUnlink(uint8_t block_, bool unlinkUp_, bool zero_ = true)
    : BlockMessage(headerId, block_)
    , data3(0x00)
  {
    setBit<zeroBit>(data3, zero_);
    setBit<unlinkBit>(data3, unlinkUp_);
  }

  bool unlinkUp() const
  {
    return getBit<unlinkBit>(data3);
  }

  bool zero() const
  {
    return getBit<zeroBit>(data3);
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(BlockUnlink)) &&
      ((message[0] & headerIdMask) == headerId) &&
      ((message[2] & 0x79) == 0x00);
  }
};
static_assert(sizeof(BlockUnlink) == 3);

struct BlockKickStart : BlockMessage
{
  static constexpr uint8_t headerId = 0x3C;

  uint8_t value;

  BlockKickStart(uint8_t block_, uint8_t value_)
    : BlockMessage(headerId, block_)
    , value(value_ & 0x7F)
  {
    assert(value < 0x7F);
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(BlockKickStart)) &&
      ((message[0] & headerIdMask) == headerId) ;
  }
};
static_assert(sizeof(BlockKickStart) == 3);

struct BlockControl : BlockMessage
{
  enum class Action : uint8_t
  {
    NoAction = 0x00,
    Clear = 0x10, // Analog/DCC is preserved
    SetAnalogLightOff = 0x40,
    SetAnalogLightOn = 0x50,
    SetDCC = 0x20,
    SetDCCAndClear = 0x30,
  };

  static constexpr uint8_t headerId = 0x3E;
  static constexpr uint8_t actionMask = 0x70;
  static constexpr uint8_t polarityMask = 0x0C;
  static constexpr uint8_t polarityPositive = 0x0C;
  static constexpr uint8_t polarityNegative = 0x08;
  static constexpr uint8_t enableDisableMask = 0x03;
  static constexpr uint8_t enable = 0x03;
  static constexpr uint8_t disable = 0x02;

  uint8_t data3; // [A D X P1 P0 U1 U0]

  BlockControl(uint8_t block_, Action action_, std::optional<Polarity> polarity_, std::optional<bool> enable_)
    : BlockMessage(headerId, block_)
    , data3(static_cast<uint8_t>(action_))
  {
    if(polarity_)
    {
      data3 |= (*polarity_ == Polarity::Positive) ? polarityPositive : polarityNegative;
    }
    if(enable_)
    {
      data3 |= *enable_ ? enable : disable;
    }
  }

  Action action() const
  {
    return static_cast<Action>(data3 & actionMask);
  }

  std::optional<Polarity> polarity() const
  {
    if((data3 & polarityMask) == polarityPositive)
    {
      return Polarity::Positive;
    }
    if((data3 & polarityMask) == polarityNegative)
    {
      return Polarity::Negative;
    }
    return std::nullopt;
  }

  std::optional<bool> enabled() const
  {
    if((data3 & enableDisableMask) == enable)
    {
      return true;
    }
    if((data3 & enableDisableMask) == disable)
    {
      return false;
    }
    return std::nullopt;
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(BlockControl)) &&
      ((message[0] & headerIdMask) == headerId) ;
  }
};
static_assert(sizeof(BlockControl) == 3);

struct InputMessage : Message
{
  static constexpr uint8_t headerIdMask = 0x60;
  static constexpr uint8_t valueBit = 4;

  uint8_t data1; // [H1 H0 V  A10 A9 A8 A7]
  uint8_t data2; // [A6 A5 A4 A3  A2 A1 A0]

  InputMessage(uint8_t headerId, uint16_t address_, bool value_)
    : data1((headerId & headerIdMask) | ((address_ >> 7) & 0x0F))
    , data2(address_ & 0x7F)
  {
    setBit<valueBit>(data1, value_);
  }

  uint16_t address() const
  {
    return (static_cast<uint16_t>(data1 & 0x0F) << 7) | (data2 & 0x7F);
  }

  bool value() const
  {
    return getBit<valueBit>(data1);
  }
};
static_assert(sizeof(InputMessage) == 2);

struct InputEvent : InputMessage
{
  static constexpr uint8_t headerId = 0x40;

  InputEvent(uint16_t address_, bool value_)
    : InputMessage(headerId, address_, value_)
  {
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(InputEvent)) &&
      ((message[0] & headerIdMask) == headerId) ;
  }
};
static_assert(sizeof(InputEvent) == 2);

struct InputRequestOrResponse : InputMessage
{
  static constexpr uint8_t headerId = 0x60;

  InputRequestOrResponse(uint16_t address_, bool value_)
    : InputMessage(headerId, address_, value_)
  {
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(InputRequestOrResponse)) &&
      ((message[0] & headerIdMask) == headerId) ;
  }
};
static_assert(sizeof(InputRequestOrResponse) == 2);

std::string toString(std::span<const uint8_t> message, bool hold, bool fault);
std::string toString(Ox32::Command cmd);

}

#endif
