/**
 * server/src/hardware/protocol/traintasticcs/messages.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_TRAINTASTICCS_MESSAGES_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_TRAINTASTICCS_MESSAGES_HPP

#include <cstdint>
#include <cstring>
#include <cstddef>
#include <string>
#include <traintastic/enum/decoderprotocol.hpp>
#include "../../../utils/byte.hpp"

namespace TraintasticCS {

struct Message;
using Checksum = std::byte;

inline Checksum calcChecksum(const Message& message);
inline bool isChecksumValid(const Message& message);
std::string toString(const Message& message);

#define FROM_CS 0x80
enum class Command : uint8_t
{
  // Traintastic -> Traintastic CS
  Reset = 0x00,
  Ping = 0x01,
  GetInfo = 0x02,

  // Traintatic CS -> Traintastic
  ResetOk = FROM_CS | Reset,
  Pong = FROM_CS | Ping,
  Info = FROM_CS | GetInfo,
  ThrottleSetSpeedDirection = FROM_CS | 0x30,
  ThrottleSetFunctions = FROM_CS | 0x31,
  Error = FROM_CS | 0x7F
};
#undef FROM_CS

constexpr std::string_view toString(Command value)
{
  switch(value)
  {
    case Command::Reset:
      return "Reset";
    case Command::Ping:
      return "Ping";
    case Command::GetInfo:
      return "GetInfo";
    case Command::ResetOk:
      return "ResetOk";
    case Command::Pong:
      return "Pong";
    case Command::Info:
      return "Info";
    case Command::ThrottleSetSpeedDirection:
      return "ThrottleSetSpeedDirection";
    case Command::ThrottleSetFunctions:
      return "ThrottleSetFunctions";
    case Command::Error:
      return "Error";
  }
  return {};
}

enum class ErrorCode : uint8_t
{
  // don't use zero, reserved for no error
  Unknown = 1,
  InvalidCommand = 2,
  InvalidCommandPayload = 3,
};

constexpr std::string_view toString(ErrorCode value)
{
  switch(value)
  {
    case ErrorCode::Unknown:
      return "Unknown";
    case ErrorCode::InvalidCommand:
      return "InvalidCommand";
    case ErrorCode::InvalidCommandPayload:
      return "InvalidCommandPayload";
  }
  return {};
}

struct Message
{
  Command command;
  uint8_t length;

  constexpr Message(Command cmd, uint8_t len)
    : command{cmd}
    , length{len}
  {
  }

  constexpr uint16_t size() const
  {
    return sizeof(Message) + length + 1;
  }
};
static_assert(sizeof(Message) == 2);

struct MessageNoData : Message
{
  Checksum checksum;

  constexpr MessageNoData(Command cmd)
    : Message(cmd, sizeof(MessageNoData) - sizeof(Message) - sizeof(checksum))
    , checksum{static_cast<Checksum>(cmd)}
  {
  }
};
static_assert(sizeof(MessageNoData) == 3);

struct Reset : MessageNoData
{
  constexpr Reset()
    : MessageNoData(Command::Reset)
  {
  }
};

struct ResetOk : MessageNoData
{
  constexpr ResetOk()
    : MessageNoData(Command::ResetOk)
  {
  }
};

struct Ping : MessageNoData
{
  constexpr Ping()
    : MessageNoData(Command::Ping)
  {
  }
};

struct Pong : MessageNoData
{
  constexpr Pong()
    : MessageNoData(Command::Pong)
  {
  }
};

struct GetInfo : MessageNoData
{
  constexpr GetInfo()
    : MessageNoData(Command::GetInfo)
  {
  }
};

enum class Board : uint8_t
{
  TraintasticCS = 1,
};

struct Info : Message
{
  Board board;
  uint8_t versionMajor;
  uint8_t versionMinor;
  uint8_t versionPatch;
  Checksum checksum;

  constexpr Info(Board board_, uint8_t major, uint8_t minor, uint8_t patch)
    : Message(Command::Info, sizeof(Info) - sizeof(Message) - sizeof(checksum))
    , board{board_}
    , versionMajor{major}
    , versionMinor{minor}
    , versionPatch{patch}
    , checksum{static_cast<Checksum>(static_cast<uint8_t>(command) ^ length ^ static_cast<uint8_t>(board) ^ versionMajor ^ versionMinor ^ versionPatch)}
  {
  }
};

enum class ThrottleChannel : uint8_t
{
  LocoNet = 1,
  XpressNet = 2,
};

struct ThrottleMessage : Message
{
  ThrottleChannel channel;
  uint8_t throttleIdH;
  uint8_t throttleIdL;
  uint8_t addressH;
  uint8_t addressL;

  uint16_t throttleId() const
  {
    return to16(throttleIdL, throttleIdH);
  }

  DecoderProtocol protocol() const
  {
    const auto value = to16(addressL, addressH);
    if((value & 0xFF80) == 0x0000)
    {
      return DecoderProtocol::DCCShort;
    }
    else if((value & 0xC000) == 0xC000)
    {
      return DecoderProtocol::DCCLong;
    }
    assert(false);
    return DecoderProtocol::None;
  }

  uint16_t address() const
  {
    const auto value = to16(addressL, addressH);
    switch(protocol())
    {
      case DecoderProtocol::DCCShort:
        return value & 0x007F;

      case DecoderProtocol::DCCLong:
        return value & 0x3FFF;

      case DecoderProtocol::None:
      case DecoderProtocol::Motorola:
      case DecoderProtocol::MFX:
      case DecoderProtocol::Selectrix:
        break;
    }
    assert(false);
    return 0;
  }
};

struct ThrottleSetSpeedDirection : ThrottleMessage
{
  uint8_t direction : 1;
  uint8_t eStop : 1;
  uint8_t setSpeedStep : 1;
  uint8_t setDirection : 1;
  uint8_t : 4;
  uint8_t speedStep;
  uint8_t speedSteps;
  Checksum checksum;
};
static_assert(sizeof(ThrottleSetSpeedDirection) == 11);

struct ThrottleSetFunctions : ThrottleMessage
{
  uint8_t functions[1];

  uint8_t functionCount() const
  {
    return length - (sizeof(ThrottleMessage) - sizeof(Message));
  }

  std::pair<uint8_t, bool> function(uint8_t index) const
  {
    return {functions[index] & 0x7F, (functions[index] & 0x80) != 0};
  }
};

struct Error : Message
{
  Command request;
  ErrorCode code;
  Checksum checksum;

  constexpr Error(Command request_, ErrorCode code_)
    : Message(Command::Error, sizeof(Error) - sizeof(Message) - sizeof(checksum))
    , request{request_}
    , code{code_}
    , checksum{static_cast<Checksum>(static_cast<uint8_t>(command) ^ length ^ static_cast<uint8_t>(request) ^ static_cast<uint8_t>(code))}
  {
  }
};
static_assert(sizeof(Error) == 5);

inline Checksum calcChecksum(const Message& message)
{
  uint8_t checksum = static_cast<uint8_t>(message.command) ^ message.length;
  const uint8_t* data = reinterpret_cast<const uint8_t*>(&message) + sizeof(Message);
  for(uint8_t i = 0; i < message.length; ++i)
  {
    checksum ^= data[i];
  }
  return static_cast<Checksum>(checksum);
}

inline bool isChecksumValid(const Message& message)
{
  return calcChecksum(message) == *(reinterpret_cast<const Checksum*>(&message) + message.length + 2);
}

}

constexpr std::string_view toString(const TraintasticCS::Board value)
{
  switch(value)
  {
    case TraintasticCS::Board::TraintasticCS:
      return "TraintasticCS";
  }
  return {};
}

constexpr std::string_view toString(const TraintasticCS::ThrottleChannel value)
{
  switch(value)
  {
    case TraintasticCS::ThrottleChannel::LocoNet:
      return "LocoNet";
    case TraintasticCS::ThrottleChannel::XpressNet:
      return "XpressNet";
  }
  return {};
}

inline bool operator ==(const TraintasticCS::Message& lhs, const TraintasticCS::Message& rhs)
{
  return lhs.size() == rhs.size() && std::memcmp(&lhs, &rhs, lhs.size()) == 0;
}

inline bool operator !=(const TraintasticCS::Message& lhs, const TraintasticCS::Message& rhs)
{
  return lhs.size() != rhs.size() || std::memcmp(&lhs, &rhs, lhs.size()) != 0;
}

#endif
