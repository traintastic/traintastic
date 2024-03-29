/**
 * server/src/lua/set.hpp - Lua set wrapper
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_LUA_SET_HPP
#define TRAINTASTIC_SERVER_LUA_SET_HPP

#include <type_traits>
#include <traintastic/set/set.hpp>
#include <lua.hpp>
#include "readonlytable.hpp"
#include "../utils/toupper.hpp"

namespace Lua {

inline void pushSet(lua_State* L, const char* setName, lua_Integer value)
{
    lua_getglobal(L, setName); // get tabel with all values: key=int, value=set as userdata
    assert(lua_type(L, -1) == LUA_TTABLE);
    lua_rawgeti(L, -1, value); // get userdata
    if(lua_isnil(L, -1)) // value not in table
    {
      lua_pop(L, 1); // remove nil
      *static_cast<lua_Integer*>(lua_newuserdata(L, sizeof(value))) = value;
      luaL_setmetatable(L, setName);
      lua_pushvalue(L, -1); // copy set userdata on stack
      lua_rawseti(L, -3, value); // add set value to table
    }
    lua_insert(L, lua_gettop(L) - 1); // swap tabel and userdata
    lua_pop(L, 1); // remove tabel
}

inline lua_Integer checkSet(lua_State* L, int index, const char* setName)
{
  return *static_cast<const lua_Integer*>(luaL_checkudata(L, index, setName));
}

inline bool testSet(lua_State* L, int index, const char* setName, lua_Integer& value)
{
  const auto* data = static_cast<const lua_Integer*>(luaL_testudata(L, index, setName));
  if(data)
    value = *data;
  return data;
}

template<typename T>
struct Set
{
  static_assert(is_set_v<T>);
  static_assert(sizeof(T) <= sizeof(lua_Integer));

  static T check(lua_State* L, int index)
  {
    return static_cast<T>(checkSet(L, index, set_name_v<T>));
  }

  static bool test(lua_State* L, int index, T& value)
  {
    lua_Integer n;
    const bool success = testSet(L, index, set_name_v<T>, n);
    if(success)
      value = static_cast<T>(n);
    return success;
  }

  static void push(lua_State* L, T value)
  {
    pushSet(L, set_name_v<T>, static_cast<lua_Integer>(value));
  }

  static int contains(lua_State* L)
  {
    lua_pushboolean(L, ::contains(check(L, lua_upvalueindex(1)), check(L, 1)) ? 1 : 0);
    return 1;
  }

  static int __index(lua_State* L)
  {
    size_t len;
    const char* s = lua_tolstring(L, 2, &len);
    std::string_view name{s, len};

    if(name == "contains")
    {
      lua_pushvalue(L, 1);
      lua_pushcclosure(L, contains, 1);
    }
    else
      lua_pushnil(L);

    return 1;
  }

  static int __add(lua_State* L)
  {
    push(L, check(L, 1) + check(L, 2));
    return 1;
  }

  static int __sub(lua_State* L)
  {
    push(L, check(L, 1) - check(L, 2));
    return 1;
  }

  static int __band(lua_State* L)
  {
    push(L, check(L, 1) & check(L, 2));
    return 1;
  }

  static int __bor(lua_State* L)
  {
    push(L, check(L, 1) | check(L, 2));
    return 1;
  }

  static int __bnot(lua_State* L)
  {
    push(L, ~check(L, 1));
    return 1;
  }

  static int __tostring(lua_State* L)
  {
    const T value = check(L, 1);
    int n = 3;
    lua_pushstring(L, set_name_v<T>);
    lua_pushliteral(L, "(");
    for(auto& it : set_values_v<T>)
      if(::contains(value, it.first))
      {
        if(n > 3)
        {
          lua_pushliteral(L, " ");
          n++;
        }
        lua_pushstring(L, toUpper(it.second).c_str());
        n++;
      }
    lua_pushliteral(L, ")");
    lua_concat(L, n);
    return 1;
  }

  static void registerType(lua_State* L)
  {
    // meta table for set userdata:
    luaL_newmetatable(L, set_name_v<T>);
    lua_pushcfunction(L, __index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, __add);
    lua_setfield(L, -2, "__add");
    lua_pushcfunction(L, __sub);
    lua_setfield(L, -2, "__sub");
    lua_pushcfunction(L, __band);
    lua_setfield(L, -2, "__band");
    lua_pushcfunction(L, __bor);
    lua_setfield(L, -2, "__bor");
    lua_pushcfunction(L, __bnot);
    lua_setfield(L, -2, "__bnot");
    lua_pushcfunction(L, __tostring);
    lua_setfield(L, -2, "__tostring");
    lua_pop(L, 1);

    // weak table for set userdata:
    lua_newtable(L);
    lua_newtable(L); // metatable
    lua_pushliteral(L, "__mode");
    lua_pushliteral(L, "v");
    lua_rawset(L, -3);
    lua_setmetatable(L, -2);
    lua_setglobal(L, set_name_v<T>);
  }

  static void registerValues(lua_State* L)
  {
    assert(lua_istable(L, -1));
    lua_createtable(L, 0, set_values_v<T>.size());
    for(auto& it : set_values_v<T>)
    {
      push(L, it.first);
      lua_setfield(L, -2, toUpper(it.second).c_str());
    }
    ReadOnlyTable::wrap(L, -1);
    lua_setfield(L, -2, set_name_v<T>);
  }
};

}

#endif
