/**
 * server/src/hardware/protocol/loconet/checksum.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022 Reinder Feenstra
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

#include "checksum.hpp"
#include "message/message.hpp"

namespace LocoNet {

uint8_t calcChecksum(const Message& message)
{
  const auto* p = reinterpret_cast<const uint8_t*>(&message);
  const int size = message.size() - 1;
  uint8_t checksum = 0xFF;
  for(int i = 0; i < size; i++)
    checksum ^= p[i];
  return checksum;
}

void updateChecksum(Message& message)
{
  reinterpret_cast<uint8_t*>(&message)[message.size() - 1] = calcChecksum(message);
}

bool isChecksumValid(const Message& message)
{
  return calcChecksum(message) == reinterpret_cast<const uint8_t*>(&message)[message.size() - 1];
}

}
