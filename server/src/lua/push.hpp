/**
 * server/src/lua/push.hpp
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

#ifndef TRAINTASTIC_SERVER_LUA_PUSH_HPP
#define TRAINTASTIC_SERVER_LUA_PUSH_HPP

#include <type_traits>
#include <string>
#include "enum.hpp"
#include "set.hpp"
#include "object.hpp"
#include "../utils/is_shared_ptr.hpp"

namespace Lua {

template<typename T>
void push(lua_State* L, const T& value)
{
  if constexpr(std::is_null_pointer_v<T>)
    lua_pushnil(L);
  else if constexpr(is_set_v<T>)
    Set<T>::push(L, value);
  else if constexpr(std::is_enum_v<T>)
    Enum<T>::push(L, value);
  else if constexpr(std::is_same_v<T, bool>)
    lua_pushboolean(L, value);
  else if constexpr(std::is_integral_v<T>)
  {
    if constexpr(std::is_unsigned_v<T> && sizeof(T) >= sizeof(lua_Integer))
    {
      if(value <= static_cast<T>(LUA_MAXINTEGER))
        lua_pushinteger(L, static_cast<lua_Integer>(value));
      else
        lua_pushnumber(L, value);
    }
    else if constexpr(std::numeric_limits<T>::min() >= LUA_MININTEGER &&
        std::numeric_limits<T>::max() <= LUA_MAXINTEGER)
      lua_pushinteger(L, value);
    else if(value >= LUA_MININTEGER && value <= LUA_MAXINTEGER)
      lua_pushinteger(L, value);
    else
      lua_pushnumber(L, value);
  }
  else if constexpr(std::is_floating_point_v<T>)
    lua_pushnumber(L, value);
  else if constexpr(std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>)
    lua_pushlstring(L, value.data(), value.size());
  else if constexpr(is_shared_ptr_v<T>)
    Object::push(L, value);
  else
    static_assert(sizeof(T) != sizeof(T), "don't know how to push type");
}

}

#endif
