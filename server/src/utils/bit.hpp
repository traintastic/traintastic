/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_UTILS_BIT_HPP
#define TRAINTASTIC_SERVER_UTILS_BIT_HPP

#include <type_traits>

template<unsigned int N, typename T>
requires(std::is_integral_v<T> && N < sizeof(T) * 8)
constexpr bool getBit(T value)
{
  return (value & (static_cast<T>(1) << N)) != 0;
}

template<unsigned int N, typename T>
requires(std::is_integral_v<T> && N < sizeof(T) * 8)
constexpr void setBit(T& value, bool set)
{
  if(set)
  {
    value |= (1 << N);
  }
  else
  {
    value &= ~(static_cast<T>(1) << N);
  }
}

#endif
