/**
 * server/src/utils/byte.hpp
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

#ifndef TRAINTASTIC_SERVER_UTILS_BYTE_HPP
#define TRAINTASTIC_SERVER_UTILS_BYTE_HPP

#include <cstdint>

constexpr uint8_t high8(const uint16_t value)
{
  return static_cast<uint8_t>(value >> 8);
}

constexpr uint8_t low8(const uint16_t value)
{
  return static_cast<uint8_t>(value & 0xFF);
}

constexpr uint16_t to16(const uint8_t valueLow, const uint8_t valueHigh)
{
  return (static_cast<uint16_t>(valueHigh) << 8) | valueLow;
}

constexpr uint16_t high16(const uint32_t value)
{
  return static_cast<uint16_t>(value >> 16);
}

constexpr uint16_t low16(const uint32_t value)
{
  return static_cast<uint16_t>(value & 0xFFFF);
}

#endif
