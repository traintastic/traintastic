/**
 * server/src/hardware/protocol/marklincan/message/statusdataconfig.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLINCAN_MESSAGE_STATUSDATACONFIG_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLINCAN_MESSAGE_STATUSDATACONFIG_HPP

#include "../messages.hpp"
#include <cstddef>
#include <vector>

namespace MarklinCAN {

struct StatusDataConfig : UidMessage
{
  StatusDataConfig(uint32_t hashUID, uint32_t uid, uint8_t index_)
    : UidMessage(hashUID, Command::StatusDataConfig, false, uid)
  {
    dlc = 5;
    setIndex(index_);
  }

  uint8_t index() const
  {
    return data[4];
  }

  void setIndex(uint8_t value)
  {
    data[4] = value;
  }
};

struct StatusDataConfigReply : UidMessage
{
  StatusDataConfigReply(uint32_t hashUID, uint32_t uid, uint8_t index_, uint8_t packetCount_)
    : UidMessage(hashUID, Command::StatusDataConfig, true, uid)
  {
    dlc = 6;
    setIndex(index_);
    setPacketCount(packetCount_);
  }

  uint8_t index() const
  {
    return data[4];
  }

  void setIndex(uint8_t value)
  {
    data[4] = value;
  }

  uint8_t packetCount() const
  {
    return data[5];
  }

  void setPacketCount(uint8_t value)
  {
    data[5] = value;
  }
};

struct StatusDataConfigReplyData : Message
{
  static constexpr uint16_t startHash = 0x301;

  StatusDataConfigReplyData(uint16_t packetNumber, const void* bytes, uint8_t byteCount)
    : Message(Command::StatusDataConfig, true)
  {
    assert(byteCount >= 1 && byteCount <= 8);
    setHash(startHash + packetNumber);
    dlc = 8;
    std::memcpy(data, bytes, byteCount);
  }
};

std::vector<Message> statusDataConfigReply(uint32_t hashUID, uint32_t uid, uint8_t index, const std::vector<std::byte>& bytes);

template<class T>
inline std::vector<Message> statusDataConfigReply(uint32_t hashUID, uint32_t uid, uint8_t index, const T& data)
{
  return statusDataConfigReply(hashUID, uid, index, data.toBytes());
}

namespace StatusData
{
  struct DeviceDescription
  {
    inline static DeviceDescription decode(const std::vector<std::byte>& bytes)
    {
      DeviceDescription desc;
      desc.fromBytes(bytes);
      return desc;
    }

    uint8_t numberOfReadings = 0;
    uint8_t numberOfConfigurationChannels = 0;
    uint16_t userDefined = 0;
    uint32_t serialNumber = 0;
    char articleNumber[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    std::string deviceName;
    std::string nickname;

    std::vector<std::byte> toBytes() const;
    void fromBytes(const std::vector<std::byte>& bytes);
  };

  struct ReadingDescription
  {
    inline static ReadingDescription decode(const std::vector<std::byte>& bytes)
    {
      ReadingDescription desc;
      desc.fromBytes(bytes);
      return desc;
    }

    uint8_t channel;
    int8_t power;
    uint8_t color[4];
    uint16_t zero;
    uint16_t rangeEnd[4];
    std::string description;
    std::string labelStart;
    std::string labelEnd;
    std::string unit;

    void fromBytes(const std::vector<std::byte>& bytes);
  };

  struct ConfigurationDescription
  {
    enum class Type : uint8_t
    {
      List = 1,
      Number = 2,
    };

    inline static ConfigurationDescription decode(const std::vector<std::byte>& bytes)
    {
      ConfigurationDescription desc;
      desc.fromBytes(bytes);
      return desc;
    }

    uint8_t channel;
    Type type;
    std::string description;

    // list:
    uint8_t default_;
    std::vector<std::string> listItems;

    // number:
    uint16_t valueMin;
    uint16_t valueMax;
    uint16_t value;
    std::string labelStart;
    std::string labelEnd;
    std::string unit;

    void fromBytes(const std::vector<std::byte>& bytes);
  };
}

}

#endif
