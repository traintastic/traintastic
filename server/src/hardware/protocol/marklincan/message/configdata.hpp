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

#include "../messages.hpp"

namespace MarklinCAN {

struct ConfigData : Message
{
  ConfigData(std::string_view name_)
    : Message(Command::ConfigData, false)
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

}

#endif
