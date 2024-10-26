/**
 * server/test/lua/set.cpp
 *
 * This file is part of the traintastic test suite.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
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

#include <catch2/catch_template_test_macros.hpp>
#include <algorithm>
#include "protect.hpp"
#include "run.hpp"
#include "../../src/lua/sets.hpp"
#include "../../src/lua/readonlytable.hpp"
#include "../../src/utils/toupper.hpp"

template<class T>
static lua_State* createState()
{
  lua_State* L = newStateWithProtect();

  Lua::Set<T>::registerType(L);

  lua_createtable(L, 0, 1);
  Lua::Set<T>::registerValues(L);
  lua_setglobal(L, "set");

  return L;
}

TEMPLATE_TEST_CASE("Lua::Set<>", "[lua][lua-set]", LUA_SETS)
{
  const TestType mask = set_mask_v<TestType>;
  const TestType emptySet = static_cast<TestType>(0);
  const TestType firstKey = set_values_v<TestType>.begin()->first;
  const TestType lastKey = set_values_v<TestType>.rbegin()->first;

  {
    INFO("write to set.*");

    lua_State* L = createState<TestType>();

    // existing keys
    for(auto& it : set_values_v<TestType>)
    {
      std::string code;
      code.assign("set.").append(set_name_v<TestType>).append(".").append(toUpper(it.second)).append(" = nil");
      REQUIRE_FALSE(run(L, code.c_str()));
      REQUIRE(lua_tostring(L, -1) == std::string_view{":1: table is readonly"});
    }

    // non existing keys
    {
      std::string code;
      code.assign("set.").append(set_name_v<TestType>).append(".non_existing_key = nil");
      REQUIRE_FALSE(run(L, code));
      REQUIRE(lua_tostring(L, -1) == std::string_view{":1: table is readonly"});
    }
    {
      std::string code;
      code.assign("set.").append(set_name_v<TestType>).append("[42] = nil");
      REQUIRE_FALSE(run(L, code));
      REQUIRE(lua_tostring(L, -1) == std::string_view{":1: table is readonly"});
    }

    closeStateWithProtect(L);
  }

{
    INFO("iterate over enum.*");

    lua_State* L = createState<TestType>();

    // load Lua baselib:
    lua_pushcfunction(L, luaopen_base);
    lua_pushliteral(L, "");
    lua_call(L, 1, 0);

    // load Lua tablelib:
    luaL_requiref(L, "table", luaopen_table, 1);

    const std::string code =
      std::string(
        "local keys = {}\n"
        "for k, v in pairs(set.") + set_name_v<TestType> + ") do\n"
        "  table.insert(keys, k)\n"
        "end\n"
        "return keys\n";
    REQUIRE(run(L, code));

    // check length
    lua_len(L, -1);
    const lua_Integer len = lua_tointeger(L, -1);
    REQUIRE(len == static_cast<lua_Integer>(set_values_v<TestType>.size()));
    lua_pop(L, 1);

    // check values (lua table -> enum values):
    for(lua_Integer i = 1; i <= len; i++)
    {
      lua_rawgeti(L, -1, i);
      REQUIRE(
        std::find_if(set_values_v<TestType>.begin(), set_values_v<TestType>.end(),
          [s=std::string_view{lua_tostring(L, -1)}](auto it)
          {
            return s == toUpper(it.second);
          }) != set_values_v<TestType>.end());
      lua_pop(L, 1);
    }

    // check values (enum values -> lua table):
    for(auto it : set_values_v<TestType>)
    {
      REQUIRE(
        [L, len, s=toUpper(it.second)]()
        {
          for(lua_Integer i = 1; i <= len; i++)
          {
            lua_rawgeti(L, -1, i);
            const bool equal = s == lua_tostring(L, -1);
            lua_pop(L, 1);
            if(equal)
              return true;
          }
          return false;
        }());
    }

    closeStateWithProtect(L);
  }

  {
    INFO("single value");

    lua_State* L = createState<TestType>();

    for(auto& it : set_values_v<TestType>)
    {
      std::string code;
      code.assign("return set.").append(set_name_v<TestType>).append(".").append(toUpper(it.second));
      REQUIRE(run(L, code));

      Lua::Set<TestType>::push(L, it.first);

      REQUIRE(lua_rawequal(L, -1, -2) == 1);
    }

    closeStateWithProtect(L);
  }

  {
    INFO("tostring");

    lua_State* L = createState<TestType>();

    // load Lua baselib (for tostring function):
    lua_pushcfunction(L, luaopen_base);
    lua_pushliteral(L, "");
    lua_call(L, 1, 0);

    // empty set
    lua_getglobal(L, "tostring");
    Lua::Set<TestType>::push(L, emptySet);
    REQUIRE(lua_pcall(L, 1, 1, 0) == LUA_OK);
    REQUIRE(lua_tostring(L, -1) == std::string(set_name_v<TestType>).append("()"));

    // set with single value
    for(auto& it : set_values_v<TestType>)
    {
      lua_getglobal(L, "tostring");
      Lua::Set<TestType>::push(L, it.first);
      REQUIRE(lua_pcall(L, 1, 1, 0) == LUA_OK);
      REQUIRE(lua_tostring(L, -1) == std::string(set_name_v<TestType>).append("(").append(toUpper(it.second)).append(")"));
    }

    // set all values
    lua_getglobal(L, "tostring");
    Lua::Set<TestType>::push(L, mask);
    REQUIRE(lua_pcall(L, 1, 1, 0) == LUA_OK);
    {
      std::string r = std::string(set_name_v<TestType>);
      r.append("(");
      for(auto& it : set_values_v<TestType>)
      {
        if(it != *set_values_v<TestType>.begin())
          r.append(" ");
        r.append(toUpper(it.second));
      }
      r.append(")");
      REQUIRE(lua_tostring(L, -1) == r);
    }
    closeStateWithProtect(L);
  }

  {
    INFO("add");

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, firstKey);
    Lua::Set<TestType>::push(L, lastKey);
    REQUIRE(protect<lua_arith>(L, LUA_OPADD));

    Lua::Set<TestType>::push(L, firstKey + lastKey);

    REQUIRE(lua_rawequal(L, -1, -2) == 1);

    closeStateWithProtect(L);
  }

  {
    INFO("subtract");

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, mask);
    Lua::Set<TestType>::push(L, lastKey);
    REQUIRE(protect<lua_arith>(L, LUA_OPSUB));

    Lua::Set<TestType>::push(L, mask - lastKey);

    REQUIRE(lua_rawequal(L, -1, -2) == 1);

    closeStateWithProtect(L);
  }

  {
    INFO("multiply");

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, mask);
    Lua::Set<TestType>::push(L, lastKey);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPMUL));

    closeStateWithProtect(L);
  }

  {
    INFO("modulo");

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, mask);
    Lua::Set<TestType>::push(L, lastKey);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPMOD));

    closeStateWithProtect(L);
  }

  {
    INFO("power");

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, mask);
    lua_pushnumber(L, 2.);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPPOW));

    closeStateWithProtect(L);
  }

  {
    INFO("divide");

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, mask);
    Lua::Set<TestType>::push(L, lastKey);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPDIV));

    closeStateWithProtect(L);
  }

  {
    INFO("divide (integer)");

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, mask);
    Lua::Set<TestType>::push(L, lastKey);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPIDIV));

    closeStateWithProtect(L);
  }

  {
    INFO("binary and");

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, firstKey);
    Lua::Set<TestType>::push(L, lastKey);
    REQUIRE(protect<lua_arith>(L, LUA_OPBAND));

    Lua::Set<TestType>::push(L, firstKey & lastKey);

    REQUIRE(lua_rawequal(L, -1, -2) == 1);

    closeStateWithProtect(L);
  }

  {
    INFO("binary or");

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, firstKey);
    Lua::Set<TestType>::push(L, lastKey);
    REQUIRE(protect<lua_arith>(L, LUA_OPBOR));

    Lua::Set<TestType>::push(L, firstKey | lastKey);

    REQUIRE(lua_rawequal(L, -1, -2) == 1);

    closeStateWithProtect(L);
  }

  {
    INFO("binary xor");

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, firstKey);
    Lua::Set<TestType>::push(L, lastKey);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPBXOR));

    closeStateWithProtect(L);
  }

  {
    INFO("shift left");

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, firstKey);
    lua_pushinteger(L, 1);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPSHL));

    closeStateWithProtect(L);
  }

  {
    INFO("shift right");

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, firstKey);
    lua_pushinteger(L, 1);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPSHR));

    closeStateWithProtect(L);
  }

  {
    INFO("unary minus");

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, firstKey);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPUNM));

    closeStateWithProtect(L);
  }

  {
    INFO("binary not");

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, firstKey);
    REQUIRE(protect<lua_arith>(L, LUA_OPBNOT));

    Lua::Set<TestType>::push(L, ~firstKey);

    REQUIRE(lua_rawequal(L, -1, -2) == 1);

    closeStateWithProtect(L);
  }
}
