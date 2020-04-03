/**
 * shared/src/set/set.hpp
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

#ifndef TRAINTASTIC_SHARED_SET_SET_HPP
#define TRAINTASTIC_SHARED_SET_SET_HPP

#include <type_traits>

template<class T>
struct is_set : std::false_type{};

template<class T>
inline constexpr bool is_set_v = is_set<T>::value;

template<class T>
struct set_name
{
  static_assert(sizeof(T) != sizeof(T), "template specialization required");
};

template<class T>
constexpr char const* set_name_v = set_name<T>::value;

template<class T>
struct set_mask
{
  static_assert(sizeof(T) != sizeof(T), "template specialization required");
};

template<class T>
constexpr T set_mask_v = set_mask<T>::value;

template<class T, std::enable_if_t<is_set_v<T>>* = nullptr>
constexpr T operator&(const T& lhs, const T& rhs)
{
  return static_cast<T>(static_cast<std::underlying_type_t<T>>(lhs) & static_cast<std::underlying_type_t<T>>(rhs));
}

template<class T, std::enable_if_t<is_set_v<T>>* = nullptr>
constexpr T operator|(const T& lhs, const T& rhs)
{
  return static_cast<T>(static_cast<std::underlying_type_t<T>>(lhs) | static_cast<std::underlying_type_t<T>>(rhs));
}

template<class T, std::enable_if_t<is_set_v<T>>* = nullptr>
constexpr T operator+(const T& lhs, const T& rhs)
{
  return lhs | rhs;
}

template<class T, std::enable_if_t<is_set_v<T>>* = nullptr>
constexpr T operator~(const T& rhs)
{
  return static_cast<T>(~static_cast<std::underlying_type_t<T>>(rhs)) & set_mask_v<T>;
}

template<class T, std::enable_if_t<is_set_v<T>>* = nullptr>
constexpr T operator-(const T& lhs, const T& rhs)
{
  return lhs & ~rhs;
}

template<class T, std::enable_if_t<is_set_v<T>>* = nullptr>
constexpr bool contains(const T& value, const T& mask)
{
  return (value & mask) == mask;
}

#endif
