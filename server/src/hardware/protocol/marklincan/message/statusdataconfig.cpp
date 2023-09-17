/**
 * server/src/hardware/protocol/marklincan/message/statusdataconfig.cpp
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

#include "statusdataconfig.hpp"

namespace MarklinCAN {

std::vector<Message> statusDataConfigReply(uint32_t hashUID, uint32_t uid, uint8_t index, const std::vector<std::byte>& bytes)
{
  const uint16_t dataMessageCount = (bytes.size() + 7) / 8;
  std::vector<Message> messages;
  messages.reserve(1 + dataMessageCount);

  const std::byte* p = bytes.data();
  size_t todo = bytes.size();
  for(uint16_t i = 0; i < dataMessageCount; i++)
  {
    const uint8_t dlc = std::min<size_t>(todo, 8);
    messages.push_back(StatusDataConfigReplyData(i, p, dlc));
    p += dlc;
    todo -= dlc;
  }
  assert(p == bytes.data() + bytes.size());

  messages.push_back(StatusDataConfigReply(hashUID, uid, index, dataMessageCount));

  return messages;
}

namespace StatusData
{
  static void append(std::byte*& p, const void* buffer, const size_t size)
  {
    p = static_cast<std::byte*>(std::memcpy(p, buffer, size)) + size;
  }

  template<typename T>
  static void append(std::byte*& p, T value)
  {
    append(p, &value, sizeof(value));
  }

  template<typename T>
  static void read(const std::byte*& p, T& value)
  {
    if constexpr(std::is_trivially_copyable_v<T>)
    {
      std::memcpy(&value, p, sizeof(value));
      p += sizeof(value);
    }
    else
      static_assert(sizeof(T) != sizeof(T));
  }

  static void read(const std::byte*& p, std::string& value, size_t maxLen)
  {
    const char* c = reinterpret_cast<const char*>(p);
    value.assign(c, strnlen(c, maxLen));
    p += value.size() + 1;
  }

  std::vector<std::byte> DeviceDescription::toBytes() const
  {
    assert(nickname.size() <= 15); // max 16 byte including NUL

    std::vector<std::byte> bytes;

    bytes.resize(
      sizeof(numberOfReadings) +
      sizeof(numberOfConfigurationChannels) +
      sizeof(userDefined) +
      sizeof(serialNumber) +
      sizeof(articleNumber) +
      deviceName.size() + 1 +
      nickname.size() + 1);

    std::byte* p = bytes.data();
    append(p, numberOfReadings);
    append(p, numberOfConfigurationChannels);
    append(p, host_to_be(userDefined));
    append(p, host_to_be(serialNumber));
    append(p, articleNumber, sizeof(articleNumber));
    append(p, deviceName.c_str(), deviceName.size() + 1);
    append(p, nickname.c_str(), nickname.size() + 1);
    assert(p == bytes.data() + bytes.size());

    return bytes;
  }

  void DeviceDescription::fromBytes(const std::vector<std::byte>& bytes)
  {
    const std::byte* p = bytes.data();
    const std::byte* const end = p + bytes.size();

    read(p, numberOfReadings);
    read(p, numberOfConfigurationChannels);
    read(p, userDefined);
    userDefined = be_to_host(userDefined);
    read(p, serialNumber);
    serialNumber = be_to_host(serialNumber);
    read(p, articleNumber);
    read(p, deviceName, end - p);
    read(p, nickname, end - p);
  }

  void ReadingDescription::fromBytes(const std::vector<std::byte>& bytes)
  {
    const std::byte* p = bytes.data();
    const std::byte* const end = p + bytes.size();

    read(p, channel);
    read(p, power);
    read(p, color);
    read(p, zero);
    zero = be_to_host(zero);
    read(p, rangeEnd);
    for(size_t i = 0; i < sizeof(rangeEnd) / sizeof(*rangeEnd); i++)
      rangeEnd[i] = be_to_host(rangeEnd[i]);
    read(p, description, end - p);
    read(p, labelStart, end - p);
    read(p, labelEnd, end - p);
    read(p, unit, end - p);
  }

  void ConfigurationDescription::fromBytes(const std::vector<std::byte>& bytes)
  {
    const std::byte* p = bytes.data();
    const std::byte* const end = p + bytes.size();

    read(p, channel);
    read(p, type);

    switch(type)
    {
      case Type::List:
      {
        uint8_t itemCount;
        read(p, itemCount);
        read(p, default_);
        uint32_t reserved;
        read(p, reserved);
        read(p, description, end - p);
        for(uint8_t i = 0; i < itemCount; i++)
        {
          std::string item;
          read(p, item, end - p);
          listItems.emplace_back(item);
        }
        break;
      }
      case Type::Number:
        read(p, valueMin);
        valueMin = be_to_host(valueMin);
        read(p, valueMax);
        valueMax = be_to_host(valueMax);
        read(p, value);
        value = be_to_host(value);
        read(p, description, end - p);
        read(p, labelStart, end - p);
        read(p, labelEnd, end - p);
        read(p, unit, end - p);
        break;
    }
  }
}

}
