/**
 * server/src/hardware/protocol/marklincan/configdatastreamcollector.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLINCAN_CONFIGDATASTREAMCOLLECTOR_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLINCAN_CONFIGDATASTREAMCOLLECTOR_HPP

#include <cstddef>
#include <vector>
#include "message/configdata.hpp"

namespace MarklinCAN {

class ConfigDataStreamCollector
{
  private:
    uint16_t m_crc = 0;
    std::vector<std::byte> m_data;
    size_t m_offset = 0;

  public:
    enum Status
    {
      Collecting,
      Complete,
      ErrorInvalidMessage,
      ErrorToMuchData,
      ErrorInvalidCRC,
    };

    const std::string name;

    ConfigDataStreamCollector(std::string name_);

    const std::byte* data() const
    {
      return m_data.data();
    }

    size_t dataSize() const
    {
      return m_data.size();
    }

    const std::vector<std::byte>& bytes() const
    {
      return m_data;
    }

    Status process(const ConfigDataStream& message);

    std::vector<std::byte>&& releaseData()
    {
      return std::move(m_data);
    }
};

}

#endif
