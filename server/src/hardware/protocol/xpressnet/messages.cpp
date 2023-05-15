/**
 * server/src/hardware/protocol/xpressnet/messages.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2022 Reinder Feenstra
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

#include "messages.hpp"
#include "../../../utils/tohex.hpp"

namespace XpressNet {

uint8_t calcChecksum(const Message& msg, const int dataSize)
{
  const uint8_t* p = reinterpret_cast<const uint8_t*>(&msg);
  uint8_t checksum = p[0];
  for(int i = 1; i <= dataSize; i++)
    checksum ^= p[i];
  return checksum;
}

void updateChecksum(Message& msg)
{
  *(reinterpret_cast<uint8_t*>(&msg) + msg.dataSize() + 1) = calcChecksum(msg);
}

bool isChecksumValid(const Message& msg, const int dataSize)
{
  return calcChecksum(msg, dataSize) == *(reinterpret_cast<const uint8_t*>(&msg) + dataSize + 1);
}

std::string toString(const Message& message, bool raw)
{
  std::string s;

  switch(message.identification())
  {
    default:
      raw = true;
      break;
  }

  if(raw)
  {
    s.append("[");
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&message);
    for(int i = 0; i < message.size(); i++)
    {
      if(i != 0)
        s.append(" ");
      s.append(toHex(bytes[i]));
    }
    s.append("]");
  }

  return s;
}

}
