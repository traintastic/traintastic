/**
 * server/src/lua/enum.hpp - Lua enum wrapper
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

#ifndef TRAINTASTIC_SERVER_LUA_ENUM_HPP
#define TRAINTASTIC_SERVER_LUA_ENUM_HPP

#include <type_traits>
#include <string_view>
#include <traintastic/enum/enum.hpp>
#include <lua.hpp>
#include "enumvalues.hpp"
#include "readonlytable.hpp"

namespace Lua {

template<typename T>
struct Enum
{
  static_assert(std::is_enum<T>::value);
  static_assert(sizeof(T) <= sizeof(lua_Integer));

  static T check(lua_State* L, int index)
  {
    return *static_cast<const T*>(luaL_checkudata(L, index, EnumName<T>::value));
  }

  static bool test(lua_State* L, int index, T& value)
  {
    const T* data = static_cast<const T*>(luaL_testudata(L, index, EnumName<T>::value));
    if(data)
      value = *data;
    return data;
  }

  static void push(lua_State* L, T value)
  {
    lua_getglobal(L, EnumName<T>::value); // get tabel with all enum values: key=int, value=userdata enum
    lua_rawgeti(L, -1, static_cast<lua_Integer>(value)); // get userdata by key
    lua_insert(L, lua_gettop(L) - 1); // swap table and userdata
    lua_pop(L, 1); // remove table
  }

  static int __tostring(lua_State* L)
  {
    lua_pushstring(L, EnumName<T>::value);
    lua_pushliteral(L, ".");
    lua_pushstring(L, EnumValues<T>::value.find(check(L, 1))->second);
    lua_concat(L, 3);
    return 1;
  }

  static void registerType(lua_State* L)
  {
    luaL_newmetatable(L, EnumName<T>::value);
    lua_pushcfunction(L, __tostring);
    lua_setfield(L, -2, "__tostring");

    lua_createtable(L, 0, EnumValues<T>::value.size());
    for(auto& it : EnumValues<T>::value)
    {
      *static_cast<T*>(lua_newuserdata(L, sizeof(T))) = it.first;
      lua_pushvalue(L, -4); // copy metatable
      lua_setmetatable(L, -2);
      lua_rawseti(L, -2, static_cast<lua_Integer>(it.first));
    }
    lua_setglobal(L, EnumName<T>::value);

    lua_pop(L, 1); // remove metatable from stack
  }

  static void registerValues(lua_State* L)
  {
    assert(lua_istable(L, -1));
    lua_createtable(L, 0, EnumValues<T>::value.size());
    for(auto& it : EnumValues<T>::value)
    {
      push(L, it.first);
      lua_setfield(L, -2, it.second);
    }
    ReadOnlyTable::setMetatable(L, -1);
    lua_setfield(L, -2, EnumName<T>::value);
  }
};

}

#endif
