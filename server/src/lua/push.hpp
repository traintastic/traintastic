/**
 * server/src/lua/push.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019 Reinder Feenstra
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

#ifndef SERVER_LUA_PUSH_HPP
#define SERVER_LUA_PUSH_HPP

#include <type_traits>
#include <string>
#include "enum.hpp"

namespace Lua {

template<typename T>
void push(lua_State* L, const T& value)
{
  if constexpr(is_enum<T>::value)
    Enum<T>::push(L, value);
  else if constexpr(std::is_same<T, bool>::value)
    lua_pushboolean(L, value);
  else if constexpr(std::is_integral<T>::value)
  {
    if constexpr(std::numeric_limits<T>::min() >= LUA_MININTEGER &&
        std::numeric_limits<T>::max() <= LUA_MAXINTEGER)
      lua_pushinteger(L, value);
    else if(value >= LUA_MININTEGER && value <= LUA_MAXINTEGER)
      lua_pushinteger(L, value);
    else
      lua_pushnumber(L, value);
  }
  else if constexpr(std::is_floating_point::value)
    lua_pushnumber(L, value);
  else if constexpr(std::is_same<T, std::string>::value)
    lua_pushlstring(L, value.data(), value.size());
  else
    static_assert(sizeof(T) != sizeof(T), "don't know how to push type");
}

}

#endif
