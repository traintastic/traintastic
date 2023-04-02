/**
 * server/src/hardware/protocol/traintasticdiy/messages.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_TRAINTASTICDIY_MESSAGES_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_TRAINTASTICDIY_MESSAGES_HPP

#include <cstring>
#include <string>
#include "opcode.hpp"
#include "inputstate.hpp"
#include "outputstate.hpp"
#include "featureflags.hpp"
#include "../../../enum/direction.hpp"
#include "../../../utils/byte.hpp"

namespace TraintasticDIY {

using Checksum = std::byte;
struct Message;

Checksum calcChecksum(const Message& message);
void updateChecksum(Message& message);
bool isChecksumValid(const Message& message);
std::string toString(const Message& message);

struct Message
{
  OpCode opCode;

  Message(OpCode opCode_)
    : opCode{opCode_}
  {
  }

  size_t dataSize() const
  {
    if(const uint8_t len = (static_cast<uint8_t>(opCode) & 0x0F); len != 0x0F)
      return len;
    return sizeof(uint8_t) + *(reinterpret_cast<const uint8_t*>(this) + 1);
  }

  size_t size() const
  {
    return sizeof(Message) + dataSize() + 1;
  }
};

struct Heartbeat : Message
{
  Checksum checksum;

  Heartbeat()
    : Message(OpCode::Heartbeat)
    , checksum{calcChecksum(*this)}
  {
  }
};
static_assert(sizeof(Heartbeat) == 2);

struct GetInputState : Message
{
  uint8_t addressHigh;
  uint8_t addressLow;
  Checksum checksum;

  GetInputState(uint16_t address_ = 0)
    : Message(OpCode::GetInputState)
    , addressHigh{high8(address_)}
    , addressLow{low8(address_)}
    , checksum{calcChecksum(*this)}
  {
  }

  uint16_t address() const
  {
    return to16(addressLow, addressHigh);
  }
};
static_assert(sizeof(GetInputState) == 4);

struct SetInputState : Message
{
  uint8_t addressHigh;
  uint8_t addressLow;
  InputState state;
  Checksum checksum;

  SetInputState(uint16_t address_, InputState state_)
    : Message(OpCode::SetInputState)
    , addressHigh{high8(address_)}
    , addressLow{low8(address_)}
    , state{state_}
    , checksum{calcChecksum(*this)}
  {
  }

  uint16_t address() const
  {
    return to16(addressLow, addressHigh);
  }
};
static_assert(sizeof(SetInputState) == 5);

struct GetOutputState : Message
{
  uint8_t addressHigh;
  uint8_t addressLow;
  Checksum checksum;

  GetOutputState(uint16_t address_ = 0)
    : Message(OpCode::GetOutputState)
    , addressHigh{high8(address_)}
    , addressLow{low8(address_)}
    , checksum{calcChecksum(*this)}
  {
  }

  uint16_t address() const
  {
    return to16(addressLow, addressHigh);
  }
};
static_assert(sizeof(GetOutputState) == 4);

struct SetOutputState  : Message
{
  uint8_t addressHigh;
  uint8_t addressLow;
  OutputState state;
  Checksum checksum;

  SetOutputState(uint16_t address_, OutputState state_)
    : Message(OpCode::SetOutputState)
    , addressHigh{high8(address_)}
    , addressLow{low8(address_)}
    , state{state_}
    , checksum{calcChecksum(*this)}
  {
  }

  uint16_t address() const
  {
    return to16(addressLow, addressHigh);
  }
};
static_assert(sizeof(SetOutputState) == 5);

struct ThrottleMessage : Message
{
  static constexpr uint8_t addressHighMask = 0x3F;

  uint8_t throttleIdHigh;
  uint8_t throttleIdLow;
  uint8_t addressHigh;
  uint8_t addressLow;

  ThrottleMessage(OpCode opCode_, uint16_t throttleId_, uint16_t address_, bool longAddress)
    : Message(opCode_)
    , throttleIdHigh{high8(throttleId_)}
    , throttleIdLow{low8(throttleId_)}
    , addressHigh((high8(address_) & addressHighMask) | (longAddress ? 0x80 : 0x00))
    , addressLow{low8(address_)}
  {
  }

  uint16_t throttleId() const
  {
    return to16(throttleIdLow, throttleIdHigh);
  }

  uint16_t address() const
  {
    return to16(addressLow, addressHigh & addressHighMask);
  }

  bool isLongAddress() const
  {
    return (addressHigh & 0x80);
  }
};
static_assert(sizeof(ThrottleMessage) == 5);

struct ThrottleSubUnsub : ThrottleMessage
{
  static constexpr uint8_t addressHighSubUnsubBit = 0x40;

  enum Action
  {
    Unsubscribe = 0,
    Subscribe = 1
  };

  Checksum checksum;

  ThrottleSubUnsub(uint16_t throttleId_, uint16_t address_, bool longAddress, Action action_)
    : ThrottleMessage(OpCode::ThrottleSubUnsub, throttleId_, address_, longAddress)
    , checksum{calcChecksum(*this)}
  {
    setAction(action_);
  }

  Action action() const
  {
    return (addressHigh & addressHighSubUnsubBit) ? Subscribe : Unsubscribe;
  }

  void setAction(Action value)
  {
    assert(value == Subscribe || value == Unsubscribe);
    switch(value)
    {
      case Subscribe:
        addressHigh |= addressHighSubUnsubBit; // set
        break;

      case Unsubscribe:
        addressHigh &= ~addressHighSubUnsubBit; // clear
        break;
    }
  }
};
static_assert(sizeof(ThrottleSubUnsub) == 6);

struct ThrottleSetSpeedDirection : ThrottleMessage
{
  static constexpr uint8_t flagDirectionForward = 0x01;
  static constexpr uint8_t flagDirectionSet = 0x40;
  static constexpr uint8_t flagSpeedSet = 0x80;

  uint8_t speed;
  uint8_t speedMax;
  uint8_t flags;
  Checksum checksum;

  ThrottleSetSpeedDirection(uint16_t throttleId_, uint16_t address_, bool longAddress, uint8_t speed_, uint8_t speedMax_, Direction direction_)
    : ThrottleMessage(OpCode::ThrottleSetSpeedDirection, throttleId_, address_, longAddress)
    , speed{speed_}
    , speedMax{speedMax_}
    , flags{flagSpeedSet | flagDirectionSet}
  {
    assert(direction_ != Direction::Unknown);
    if(direction_ == Direction::Forward)
      flags |= flagDirectionForward;
    updateChecksum(*this);
  }

  ThrottleSetSpeedDirection(uint16_t throttleId_, uint16_t address_, bool longAddress, uint8_t speed_, uint8_t speedMax_)
    : ThrottleMessage(OpCode::ThrottleSetSpeedDirection, throttleId_, address_, longAddress)
    , speed{speed_}
    , speedMax{speedMax_}
    , flags{flagSpeedSet}
    , checksum{calcChecksum(*this)}
  {
  }

  ThrottleSetSpeedDirection(uint16_t throttleId_, uint16_t address_, bool longAddress, Direction direction_)
    : ThrottleMessage(OpCode::ThrottleSetSpeedDirection, throttleId_, address_, longAddress)
    , speed{0}
    , speedMax{0}
    , flags{flagDirectionSet}
  {
    assert(direction_ != Direction::Unknown);
    if(direction_ == Direction::Forward)
      flags |= flagDirectionForward;
    updateChecksum(*this);
  }

  bool isEmergencyStop() const
  {
    return speedMax == 0;
  }

  float throttle() const
  {
    return (speedMax > 0) ? std::min<float>(static_cast<float>(speed) / static_cast<float>(speedMax), 1) : 0;
  }

  bool isSpeedSet() const
  {
    return (flags & flagSpeedSet);
  }

  bool isDirectionSet() const
  {
    return (flags & flagDirectionSet);
  }

  Direction direction() const
  {
    return (flags & flagDirectionForward) ? Direction::Forward : Direction::Reverse;
  }
};
static_assert(sizeof(ThrottleSetSpeedDirection) == 9);

struct ThrottleSetFunction : ThrottleMessage
{
  static constexpr uint8_t functionNumberMask = 0x7F;
  static constexpr uint8_t functionValueMask = 0x80;
  static constexpr uint8_t functionValueOn = 0x80;
  static constexpr uint8_t functionValueOff = 0x00;

  uint8_t function;
  Checksum checksum;

  ThrottleSetFunction(uint16_t throttleId_, uint16_t address_, bool longAddress, uint8_t functionNumber_, bool functionValue_)
    : ThrottleMessage(OpCode::ThrottleSetFunction, throttleId_, address_, longAddress)
    , function((functionNumber_ & functionNumberMask) | (functionValue_ ? functionValueOn : functionValueOff))
    , checksum{calcChecksum(*this)}
  {
  }

  uint8_t functionNumber() const
  {
    return function & functionNumberMask;
  }

  bool functionValue() const
  {
    return (function & functionValueMask) == functionValueOn;
  }
};
static_assert(sizeof(ThrottleSetFunction) == 7);

struct GetFeatures  : Message
{
  Checksum checksum;

  GetFeatures()
    : Message(OpCode::GetFeatures)
    , checksum{calcChecksum(*this)}
  {
  }
};
static_assert(sizeof(GetFeatures) == 2);

struct Features : Message
{
  FeatureFlags1 featureFlags1;
  FeatureFlags2 featureFlags2;
  FeatureFlags3 featureFlags3;
  FeatureFlags4 featureFlags4;
  Checksum checksum;

  Features(FeatureFlags1 ff1 = FeatureFlags1::None, FeatureFlags2 ff2 = FeatureFlags2::None, FeatureFlags3 ff3 = FeatureFlags3::None, FeatureFlags4 ff4 = FeatureFlags4::None)
    : Message(OpCode::Features)
    , featureFlags1{ff1}
    , featureFlags2{ff2}
    , featureFlags3{ff3}
    , featureFlags4{ff4}
    , checksum{calcChecksum(*this)}
  {
  }
};
static_assert(sizeof(Features) == 6);

struct GetInfo : Message
{
  Checksum checksum;

  GetInfo()
    : Message(OpCode::GetInfo)
    , checksum{calcChecksum(*this)}
  {
  }
};
static_assert(sizeof(GetInfo) == 2);

struct InfoBase : Message
{
  uint8_t length;

  std::string_view text() const
  {
    return {reinterpret_cast<const char*>(this) + sizeof(Message) + sizeof(length), length};
  }
};
static_assert(sizeof(InfoBase) == 2);

}

inline bool operator ==(const TraintasticDIY::Message& lhs, const TraintasticDIY::Message& rhs)
{
  return lhs.size() == rhs.size() && std::memcmp(&lhs, &rhs, lhs.size()) == 0;
}

inline bool operator !=(const TraintasticDIY::Message& lhs, const TraintasticDIY::Message& rhs)
{
  return lhs.size() != rhs.size() || std::memcmp(&lhs, &rhs, lhs.size()) != 0;
}

#endif
