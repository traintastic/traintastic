/**
 * server/src/hardware/protocol/marklincan/message/configdata.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLINCAN_MESSAGE_CONFIGDATA_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLINCAN_MESSAGE_CONFIGDATA_HPP

#include <cstddef>
#include <vector>
#include "../messages.hpp"

namespace MarklinCAN {

namespace ConfigDataName {
  constexpr std::string_view loks = "loks";
}

struct ConfigData : Message
{
  ConfigData(uint32_t hashUID, std::string_view name_, bool response = false)
    : Message(hashUID, Command::ConfigData, response)
  {
    assert(name_.size() <= 8);
    dlc = 8;
    std::memcpy(data, name_.data(), std::min<size_t>(name_.size(), 8));
  }

  std::string_view name() const
  {
    const char* str = reinterpret_cast<const char*>(data);
    return {str, strnlen(str, dlc)};
  }
};

struct ConfigDataStream : Message
{
  ConfigDataStream(uint32_t hashUID, uint32_t length_, uint16_t crc_)
    : Message(hashUID, Command::ConfigDataStream, false)
  {
    dlc = 6;
    *reinterpret_cast<uint32_t*>(data) = host_to_be(length_);
    *reinterpret_cast<uint16_t*>(data + sizeof(uint32_t)) = host_to_be(crc_);
  }

  ConfigDataStream(uint32_t hashUID, const void* buffer, size_t size)
    : Message(hashUID, Command::ConfigDataStream, false)
  {
    dlc = 8;
    std::memcpy(data, buffer, std::min<size_t>(size, dlc));
  }

  bool isStart() const
  {
    return dlc == 6 || dlc == 7;
  }

  uint32_t length() const
  {
    assert(isStart());
    return be_to_host(*reinterpret_cast<const uint32_t*>(data));
  }

  uint16_t crc() const
  {
    assert(isStart());
    return be_to_host(*reinterpret_cast<const uint16_t*>(data + sizeof(uint32_t)));
  }

  bool isData() const
  {
    return dlc == 8;
  }
};

uint16_t crc16(const std::vector<std::byte>& data);

}

#endif
