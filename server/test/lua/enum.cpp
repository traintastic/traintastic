/**
 * server/test/lua/enum.cpp
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
#include "../../src/lua/enums.hpp"
#include "../../src/lua/enum.hpp"
#include "../../src/lua/readonlytable.hpp"
#include "../../src/utils/toupper.hpp"

template<class T>
static lua_State* createState()
{
  lua_State* L = newStateWithProtect();

  Lua::Enum<T>::registerType(L);

  lua_createtable(L, 0, 1);
  Lua::Enum<T>::registerValues(L);
  lua_setglobal(L, "enum");

  return L;
}

TEMPLATE_TEST_CASE("Lua::Enum<>", "[lua][lua-enum]", LUA_ENUMS)
{
  const TestType firstKey = EnumValues<TestType>::value.begin()->first;
  const TestType lastKey = EnumValues<TestType>::value.rbegin()->first;

  {
    INFO("write to enum.*");

    lua_State* L = createState<TestType>();

    // existing keys
    for(auto& it : EnumValues<TestType>::value)
    {
      std::string code;
      code.assign("enum.").append(EnumName<TestType>::value).append(".").append(toUpper(it.second)).append(" = nil");
      REQUIRE_FALSE(run(L, code.c_str()));
      REQUIRE(lua_tostring(L, -1) == std::string_view{":1: table is readonly"});
    }

    // non existing keys
    {
      std::string code;
      code.assign("enum.").append(EnumName<TestType>::value).append(".non_existing_key = nil");
      REQUIRE_FALSE(run(L, code));
      REQUIRE(lua_tostring(L, -1) == std::string_view{":1: table is readonly"});
    }
    {
      std::string code;
      code.assign("enum.").append(EnumName<TestType>::value).append("[42] = nil");
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
        "for k, v in pairs(enum.") + EnumName<TestType>::value + ") do\n"
        "  table.insert(keys, k)\n"
        "end\n"
        "return keys\n";
    REQUIRE(run(L, code));

    // check length
    lua_len(L, -1);
    const lua_Integer len = lua_tointeger(L, -1);
    REQUIRE(len == static_cast<lua_Integer>(EnumValues<TestType>::value.size()));
    lua_pop(L, 1);

    // check values (lua table -> enum values):
    for(lua_Integer i = 1; i <= len; i++)
    {
      lua_rawgeti(L, -1, i);
      REQUIRE(
        std::find_if(EnumValues<TestType>::value.begin(), EnumValues<TestType>::value.end(),
          [s=std::string_view{lua_tostring(L, -1)}](auto it)
          {
            return s == toUpper(it.second);
          }) != EnumValues<TestType>::value.end());
      lua_pop(L, 1);
    }

    // check values (enum values -> lua table):
    for(auto it : EnumValues<TestType>::value)
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

    for(auto& it : EnumValues<TestType>::value)
    {
      std::string code;
      code.assign("return enum.").append(EnumName<TestType>::value).append(".").append(toUpper(it.second));
      REQUIRE(run(L, code));

      Lua::Enum<TestType>::push(L, it.first);

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

    for(auto& it : EnumValues<TestType>::value)
    {
      lua_getglobal(L, "tostring");
      Lua::Enum<TestType>::push(L, it.first);
      REQUIRE(lua_pcall(L, 1, 1, 0) == LUA_OK);

      REQUIRE(lua_tostring(L, -1) == std::string(EnumName<TestType>::value).append(".").append(toUpper(it.second)));
    }

    closeStateWithProtect(L);
  }

  {
    INFO("add");

    lua_State* L = createState<TestType>();

    Lua::Enum<TestType>::push(L, firstKey);
    Lua::Enum<TestType>::push(L, lastKey);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPADD));

    closeStateWithProtect(L);
  }

  {
    INFO("subtract");

    lua_State* L = createState<TestType>();

    Lua::Enum<TestType>::push(L, firstKey);
    Lua::Enum<TestType>::push(L, lastKey);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPSUB));

    closeStateWithProtect(L);
  }

  {
    INFO("multiply");

    lua_State* L = createState<TestType>();

    Lua::Enum<TestType>::push(L, firstKey);
    Lua::Enum<TestType>::push(L, lastKey);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPMUL));

    closeStateWithProtect(L);
  }

  {
    INFO("modulo");

    lua_State* L = createState<TestType>();

    Lua::Enum<TestType>::push(L, firstKey);
    Lua::Enum<TestType>::push(L, lastKey);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPMOD));

    closeStateWithProtect(L);
  }

  {
    INFO("power");

    lua_State* L = createState<TestType>();

    Lua::Enum<TestType>::push(L, firstKey);
    lua_pushnumber(L, 2.);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPPOW));

    closeStateWithProtect(L);
  }

  {
    INFO("divide");

    lua_State* L = createState<TestType>();

    Lua::Enum<TestType>::push(L, firstKey);
    Lua::Enum<TestType>::push(L, lastKey);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPDIV));

    closeStateWithProtect(L);
  }

  {
    INFO("divide (integer)");

    lua_State* L = createState<TestType>();

    Lua::Enum<TestType>::push(L, firstKey);
    Lua::Enum<TestType>::push(L, lastKey);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPIDIV));

    closeStateWithProtect(L);
  }

  {
    INFO("binary and");

    lua_State* L = createState<TestType>();

    Lua::Enum<TestType>::push(L, firstKey);
    Lua::Enum<TestType>::push(L, lastKey);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPBAND));

    closeStateWithProtect(L);
  }

  {
    INFO("binary or");

    lua_State* L = createState<TestType>();

    Lua::Enum<TestType>::push(L, firstKey);
    Lua::Enum<TestType>::push(L, lastKey);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPBOR));

    closeStateWithProtect(L);
  }

  {
    INFO("binary xor");

    lua_State* L = createState<TestType>();

    Lua::Enum<TestType>::push(L, firstKey);
    Lua::Enum<TestType>::push(L, lastKey);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPBXOR));

    closeStateWithProtect(L);
  }

  {
    INFO("shift left");

    lua_State* L = createState<TestType>();

    Lua::Enum<TestType>::push(L, firstKey);
    lua_pushinteger(L, 1);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPSHL));

    closeStateWithProtect(L);
  }

  {
    INFO("shift right");

    lua_State* L = createState<TestType>();

    Lua::Enum<TestType>::push(L, firstKey);
    lua_pushinteger(L, 1);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPSHR));

    closeStateWithProtect(L);
  }

  {
    INFO("unary minus");

    lua_State* L = createState<TestType>();

    Lua::Enum<TestType>::push(L, firstKey);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPUNM));

    closeStateWithProtect(L);
  }

  {
    INFO("binary not");

    lua_State* L = createState<TestType>();

    Lua::Enum<TestType>::push(L, firstKey);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPBNOT));

    closeStateWithProtect(L);
  }
}
