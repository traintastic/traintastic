/**
 * shared/src/traintastic/utils/valuetypetraits.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021,2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_UTILS_VALUETYPETRAITS_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_UTILS_VALUETYPETRAITS_HPP

#include "../enum/valuetype.hpp"
#include "../set/set.hpp"

class Object;

template<class T>
struct value_type
{
  static constexpr ValueType value =
    std::is_same_v<T, bool> ? ValueType::Boolean : (
    is_set_v<T> ? ValueType::Set : (
    std::is_enum_v<T> ? ValueType::Enum : (
    std::is_integral_v<T> ? ValueType::Integer : (
    std::is_floating_point_v<T> ? ValueType::Float : (
#ifdef QT_CORE_LIB
    std::is_same_v<T, QString> ? ValueType::String : (
#else
    (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>) ? ValueType::String : (
#endif
    ValueType::Invalid))))));
};

template<class T>
struct value_type<std::shared_ptr<T>>
{
  static constexpr ValueType value = std::is_base_of_v<Object, T> ? ValueType::Object : ValueType::Invalid;
};

template<class T>
struct value_type<const std::shared_ptr<T>&>
{
  static constexpr ValueType value = std::is_base_of_v<Object, T> ? ValueType::Object : ValueType::Invalid;
};

template<typename T>
inline constexpr ValueType value_type_v = value_type<T>::value;

template<typename T>
constexpr bool valueTypeByRef()
{
  const auto vt = value_type_v<T>;
  return
    vt == ValueType::String ||
    vt == ValueType::Object;
}

#endif
