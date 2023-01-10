/**
 * server/src/utils/endian.hpp
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

#ifndef TRAINTASTIC_SERVER_UTILS_ENDIAN_HPP
#define TRAINTASTIC_SERVER_UTILS_ENDIAN_HPP

#include <type_traits>


#if defined(__MINGW32__) || defined(__MINGW64__)
//Needed for definition of _byteswap_ushort, _byteswap_ulong and _byteswap_uint64 on MinGW
#include <cstdlib>
#endif

#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)
  #define bswap_16(x) _byteswap_ushort(x)
  #define bswap_32(x) _byteswap_ulong(x)
  #define bswap_64(x) _byteswap_uint64(x)
#elif defined(__linux__)
  #include <byteswap.h>
#elif defined(__APPLE__)
  #include <libkern/OSByteOrder.h>
  #define bswap_16(x) OSSwapInt16(x)
  #define bswap_32(x) OSSwapInt32(x)
  #define bswap_64(x) OSSwapInt64(x)
#endif

constexpr bool is_big_endian = false;
constexpr bool is_little_endian = true;

template<typename T>
inline T byte_swap(T value)
{
  static_assert(std::is_integral_v<T> || std::is_enum_v<T>);
  if constexpr(sizeof(T) == 2)
    return bswap_16(value);
  else if constexpr(sizeof(T) == 4)
    return bswap_32(value);
  else if constexpr(sizeof(T) == 8)
    return bswap_64(value);
  else
    static_assert(sizeof(T) != sizeof(T));
}

template<typename T>
constexpr T host_to_le(T value)
{
  static_assert(std::is_integral_v<T> || std::is_enum_v<T>);
  static_assert(sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);

  if constexpr(is_big_endian/*_v<sizeof(T)>*/)
    return byte_swap(value);
  else if constexpr(is_little_endian/*_v<sizeof(T)>*/)
    return value;
  else
    static_assert(sizeof(T) != sizeof(T));
}

template<typename T>
constexpr T le_to_host(T value)
{
  static_assert(std::is_integral_v<T> || std::is_enum_v<T>);
  static_assert(sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);

  if constexpr(is_big_endian/*_v<sizeof(T)>*/)
    return byte_swap(value);
  else if constexpr(is_little_endian/*_v<sizeof(T)>*/)
    return value;
  else
    static_assert(sizeof(T) != sizeof(T));
}

template<typename T>
constexpr T host_to_be(T value)
{
  static_assert(std::is_integral_v<T> || std::is_enum_v<T>);
  static_assert(sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);

  if constexpr(is_little_endian/*_v<sizeof(T)>*/)
    return byte_swap(value);
  else if constexpr(is_big_endian/*_v<sizeof(T)>*/)
    return value;
  else
    static_assert(sizeof(T) != sizeof(T));
}

template<typename T>
constexpr T be_to_host(T value)
{
  static_assert(std::is_integral_v<T> || std::is_enum_v<T>);
  static_assert(sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);

  if constexpr(is_little_endian/*_v<sizeof(T)>*/)
    return byte_swap(value);
  else if constexpr(is_big_endian/*_v<sizeof(T)>*/)
    return value;
  else
    static_assert(sizeof(T) != sizeof(T));
}

#endif
