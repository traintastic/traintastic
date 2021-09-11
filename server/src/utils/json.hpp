/**
 * server/src/utils/json.hpp
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

#ifndef TRAINTASTIC_SERVER_UTILS_JSON_HPP
#define TRAINTASTIC_SERVER_UTILS_JSON_HPP

#include <nlohmann/json.hpp>
#include <traintastic/enum/enum.hpp>
#include <traintastic/set/set.hpp>

template<typename BasicJsonType, typename EnumType>
inline void to_json(BasicJsonType& j, EnumType e, typename std::enable_if_t<std::is_enum_v<EnumType> && !is_set_v<EnumType>>* = nullptr)
{
  j = EnumValues<EnumType>::value.at(e);
}

template<typename BasicJsonType, typename EnumType>
inline void from_json(const BasicJsonType& j, EnumType& e, typename std::enable_if_t<std::is_enum_v<EnumType> && !is_set_v<EnumType>>* = nullptr)
{
  auto it = std::find_if(EnumValues<EnumType>::value.cbegin(), EnumValues<EnumType>::value.cend(),
    [j](const auto& v)
    {
      return v.second == j;
    });

  e = it != EnumValues<EnumType>::value.cend() ? it->first : EnumValues<EnumType>::value.cbegin()->first;
}

template<typename BasicJsonType, typename EnumType>
inline void to_json(BasicJsonType& /*j*/, const EnumType& /*e*/, typename std::enable_if_t<is_set_v<EnumType>>* = nullptr)
{
  assert(false);
}

template<typename BasicJsonType, typename EnumType>
inline void from_json(const BasicJsonType& /*j*/, EnumType& /*e*/, typename std::enable_if_t<is_set_v<EnumType>>* = nullptr)
{
  assert(false);
}

#endif
