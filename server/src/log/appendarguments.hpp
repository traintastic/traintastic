/**
 * server/src/log/appendarguments.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_LOG_APPENDARGUMENTS_HPP
#define TRAINTASTIC_SERVER_LOG_APPENDARGUMENTS_HPP

#include <vector>
#include <string>
#include <type_traits>
#include <stdexcept>
#include <boost/system/error_code.hpp>
#include <traintastic/utils/stdfilesystem.hpp>
#include "../core/object.hpp"

template<class T, class... Ts>
inline static void appendArguments(std::vector<std::string>& list, const T& value, const Ts&... others)
{
  if constexpr(std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view> || std::is_same_v<T, const char*>)
    list.emplace_back(value);
  else if constexpr((std::is_integral_v<T> && !std::is_enum_v<T>) || std::is_floating_point_v<T>)
    list.emplace_back(std::to_string(value));
  else if constexpr(std::is_same_v<T, boost::system::error_code> || std::is_same_v<T, std::error_code>)
    list.emplace_back(value.message());
  else if constexpr(std::is_same_v<T, std::filesystem::path>)
    list.emplace_back(value.string());
  else if constexpr(std::is_base_of_v<Object, T>)
    list.emplace_back(value.getObjectId());
  else if constexpr(std::is_base_of_v<std::exception, T>)
    list.emplace_back(value.what());
  else
    list.emplace_back(toString(value));

  if constexpr(sizeof...(Ts) > 0)
    appendArguments(list, std::forward<const Ts&>(others)...);
}

#endif
