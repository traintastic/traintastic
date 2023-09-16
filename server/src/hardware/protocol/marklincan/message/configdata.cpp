/**
 * server/src/hardware/protocol/marklincan/message/configdata.cpp
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

#include "configdata.hpp"

namespace MarklinCAN {

//! CRC algorithm as documented by Marklin in the CS2 CAN documentation
static uint16_t crc16(uint16_t crc, uint8_t value)
{
  constexpr uint16_t poly = 0x1021;

  // Create the CRC "dividend" for polynomial arithmetic (binary arithmetic with no carries)
  crc = crc ^ (static_cast<uint16_t>(value) << 8);

  // "Divide" the poly into the dividend using CRC XOR subtraction CRC_acc holds the
  // "remainder" of each divide. Only complete this division for 8 bits since input is 1 byte
  for (int i = 0; i < 8; i++)
  {
    // Check if the MSB is set (if MSB is 1, then the POLY can "divide" into the "dividend")
    if((crc & 0x8000) == 0x8000)
    {
      // if so, shift the CRC value, and XOR "subtract" the poly
      crc = crc << 1;
      crc ^= poly;
    }
    else
    {
      // if not, just shift the CRC value
      crc = crc << 1;
    }
  }

  // Return the final remainder (CRC value)
  return crc;
}

uint16_t crc16(const std::vector<std::byte>& data)
{
  uint16_t crc = 0xFFFF;

  for(auto value : data)
  {
    crc = crc16(crc, static_cast<uint8_t>(value));
  }

  if(data.size() % 8 != 0)
  {
    // unused nul bytes must be included in CRC:
    const size_t n = 8 - (data.size() % 8);
    for(size_t i = 0; i < n; i++)
      crc = crc16(crc, 0x00);
  }

  return crc;
}

}
