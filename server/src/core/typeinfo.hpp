/**
 * server/src/core/typeinfo.hpp
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

#ifndef TRAINTASTIC_SERVER_CORE_TYPEINFO_HPP
#define TRAINTASTIC_SERVER_CORE_TYPEINFO_HPP

#include <array>
#include <string_view>
#include <traintastic/enum/valuetype.hpp>
#include <traintastic/set/set.hpp>

struct TypeInfo
{
  ValueType type;
  std::string_view enumName;
  std::string_view setName;
};

template<class T>
static constexpr TypeInfo getTypeInfo()
{
  using A = std::remove_const_t<std::remove_reference_t<T>>;
  if constexpr(value_type_v<A> == ValueType::Set)
    return {ValueType::Set, {}, set_name_v<A>};
  else if constexpr(value_type_v<A> == ValueType::Enum)
    return {ValueType::Enum, EnumName<A>::value, {}};
  else
    return {value_type_v<A>, {}, {}};
}

template<class... A>
struct TypeInfoArray
{
  static constexpr std::array<TypeInfo, sizeof...(A)> value = {{getTypeInfo<A>()...}};
};

template<class... A>
inline constexpr auto typeInfoArray = TypeInfoArray<A...>::value;

#endif
