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
  Ping = 0x00,
  GetInfo = 0x01,

  // Traintatic CS -> Traintastic
  Pong = FROM_CS | Ping,
  Info = FROM_CS | GetInfo,
};
#undef FROM_CS

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

inline bool operator ==(const TraintasticCS::Message& lhs, const TraintasticCS::Message& rhs)
{
  return lhs.size() == rhs.size() && std::memcmp(&lhs, &rhs, lhs.size()) == 0;
}

inline bool operator !=(const TraintasticCS::Message& lhs, const TraintasticCS::Message& rhs)
{
  return lhs.size() != rhs.size() || std::memcmp(&lhs, &rhs, lhs.size()) != 0;
}

#endif
