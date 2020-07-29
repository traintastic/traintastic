/**
 * server/src/utils/to_hex.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_UTILS_TO_HEX_HPP
#define TRAINTASTIC_SERVER_UTILS_TO_HEX_HPP

#include <string>

template<typename T>
std::string to_hex(T value, size_t length = sizeof(T) * 2)
{
  static const char* digits = "0123456789ABCDEF";
  std::string s(length, '0');
  for(size_t i = 0, j = length - 1; i < length; i++, j--)
    s[j] = digits[(value >> (i * 4)) & 0xf];
  return s;
}

#endif
