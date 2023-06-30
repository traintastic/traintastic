/**
 * server/src/log/joinarguments.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_LOG_JOINARGUMENTS_HPP
#define TRAINTASTIC_SERVER_LOG_JOINARGUMENTS_HPP

#include <vector>
#include <string>
#include <type_traits>
#include <stdexcept>
#include <boost/system/error_code.hpp>
#include <traintastic/utils/stdfilesystem.hpp>
#include "../core/object.hpp"

template<class T, class... Ts>
inline static void joinArguments(std::string& s, const T& value, const Ts&... others)
{
  if constexpr(std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view> || std::is_same_v<T, const char*> ||
      (std::is_array_v<T> && std::is_same_v<std::remove_all_extents_t<T>, char>))
    s.append(value);
  else if constexpr((std::is_integral_v<T> && !std::is_enum_v<T>) || std::is_floating_point_v<T>)
    s.append(std::to_string(value));
  else if constexpr(std::is_same_v<T, boost::system::error_code> || std::is_same_v<T, std::error_code>)
    s.append(value.message());
  else if constexpr(std::is_same_v<T, std::filesystem::path>)
    s.append(value.string());
  else if constexpr(std::is_base_of_v<Object, T>)
    s.append(value.getObjectId());
  else if constexpr(std::is_base_of_v<std::exception, T>)
    s.append(value.what());
  else
    s.append(toString(value));

  if constexpr(sizeof...(Ts) > 0)
    joinArguments(s.append(" "), std::forward<const Ts&>(others)...);
}

#endif
