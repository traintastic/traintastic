/**
 * server/src/utils/tohex.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#include "tohex.hpp"
#include <cstdint>

std::string toHex(const void* buffer, const size_t size)
{
  std::string s;
  s.reserve(size * 2);
  const uint8_t* p = reinterpret_cast<const uint8_t*>(buffer);
  for(size_t i = 0; i < size; i++, p++)
  {
    s.push_back(toHexDigits[*p >> 4]);
    s.push_back(toHexDigits[*p & 0x0F]);
  }
  return s;
}
