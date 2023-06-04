/**
 * server/src/lua/check.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021,2023 Reinder Feenstra
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
#include "enum.hpp"
#include "set.hpp"
#include "metatable.hpp"
#include "../core/object.hpp"
#include "../utils/startswith.hpp"

namespace Lua {

template<typename T>
std::conditional_t<std::is_base_of_v<::Object, T>, std::shared_ptr<T>, T> check(lua_State* L, int index)
{
  if constexpr(std::is_same_v<T, bool>)
  {
    luaL_checktype(L, index, LUA_TBOOLEAN);
    return lua_toboolean(L, index);
  }
  else if constexpr(is_set_v<T>)
  {
    return Set<T>::check(L, index);
  }
  else if constexpr(std::is_enum_v<T>)
  {
    return Enum<T>::check(L, index);
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
  else if constexpr(std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>)
  {
    size_t l;
    const char* s = luaL_checklstring(L, index, &l);
    return T{s, l};
  }
  else if constexpr(std::is_same_v<T, ::Object> || std::is_base_of_v<::Object, T>)
  {
    auto name = MetaTable::getName(L, index);
    if(name == "object" || startsWith(name, "object."))
    {
      auto object = static_cast<ObjectPtrWeak*>(lua_touserdata(L, index))->lock();

      if(!object)
        errorDeadObject(L);

      if constexpr(std::is_same_v<T, ::Object>)
        return object;

      if(auto objectT = std::dynamic_pointer_cast<T>(object))
        return objectT;

      errorArgumentInvalidObject(L, index);
    }
    errorArgumentExpectedObject(L, index);
  }
  else
    static_assert(sizeof(T) != sizeof(T), "don't know how to check type");
}

}

#endif
