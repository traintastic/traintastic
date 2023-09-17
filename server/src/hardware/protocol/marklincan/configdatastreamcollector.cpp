/**
 * server/src/hardware/protocol/marklincan/configdatastreamcollector.cpp
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

#include "configdatastreamcollector.hpp"

namespace MarklinCAN {

ConfigDataStreamCollector::ConfigDataStreamCollector(std::string name_)
  : name{std::move(name_)}
{
}

ConfigDataStreamCollector::Status ConfigDataStreamCollector::process(const ConfigDataStream& message)
{
  if(message.isData() && m_crc != 0x0000)
  {
    if(m_offset >= m_data.size()) /*[[unlikely]]*/
      return ErrorToMuchData;

    if(m_offset + 8 >= m_data.size()) // last message
    {
      std::memcpy(m_data.data() + m_offset, message.data, m_data.size() - m_offset);
      m_offset = m_data.size();

      if(crc16(m_data) != m_crc)
        return ErrorInvalidCRC;

      return Complete;
    }

    std::memcpy(m_data.data() + m_offset, message.data, 8);
    m_offset += 8;
    return Collecting;
  }
  else if(message.isStart() && m_crc == 0x0000)
  {
    m_data.resize(message.length());
    m_crc = message.crc();
    return Collecting;
  }

  return ErrorInvalidMessage;
}

}
