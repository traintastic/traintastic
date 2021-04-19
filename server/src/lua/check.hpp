/**
 * server/src/lua/check.hpp
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

#ifndef TRAINTASTIC_SERVER_LUA_CHECK_HPP
#define TRAINTASTIC_SERVER_LUA_CHECK_HPP

#include <lua.hpp>
#include <type_traits>
#include <cmath>
#include "error.hpp"

namespace Lua {

template<typename T>
T check(lua_State* L, int index)
{
  if constexpr(std::is_same_v<T, bool>)
  {
    luaL_checktype(L, index, LUA_TBOOLEAN);
    return lua_toboolean(L, index);
  }
  else if constexpr(std::is_integral_v<T>)
  {
    const lua_Integer value = luaL_checkinteger(L, index);
    if constexpr(std::is_unsigned_v<T> && sizeof(T) >= sizeof(value))
    {
      if(value >= 0)
        return static_cast<T>(value);
    }
    else if constexpr(std::numeric_limits<T>::min() <= LUA_MININTEGER && std::numeric_limits<T>::max() >= LUA_MAXINTEGER)
      return value;
    else if(value >= std::numeric_limits<T>::min() && value <= std::numeric_limits<T>::max())
      return value;

    errorArgumentOutOfRange(L, index);
  }
  else if constexpr(std::is_floating_point_v<T>)
    return luaL_checknumber(L, index);
  else if constexpr(std::is_same_v<T, std::string>)
    return luaL_checkstring(L, index);
  //else if constexpr(std::is_same_v<T, ObjectPtr>)
   // Object::push(L, value);
  else
    static_assert(sizeof(T) != sizeof(T), "don't know how to check type");
}

}

#endif
