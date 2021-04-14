/**
 * server/test/lua/set.cpp
 *
 * This file is part of the traintastic test suite.
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

#include <catch2/catch.hpp>
#include "protect.hpp"
#include "../../src/lua/set.hpp"
#include "../../src/utils/toupper.hpp"

// Sets:
#include "../../src/set/worldstate.hpp"

template<class T>
static lua_State* createState()
{
  lua_State* L = luaL_newstate();

  Lua::Set<T>::registerType(L);

  lua_createtable(L, 0, 1);
  Lua::Set<T>::registerValues(L);
  lua_setglobal(L, "set");

  return L;
}

TEMPLATE_TEST_CASE("Lua::Set<>", "[lua][lua-set]", WorldState)
{
  const TestType mask = set_mask_v<TestType>;
  const TestType firstKey = Lua::set_values_v<TestType>.begin()->first;
  const TestType lastKey = Lua::set_values_v<TestType>.rbegin()->first;

  {
    INFO("single value")

    lua_State* L = createState<TestType>();

    for(auto& it : Lua::set_values_v<TestType>)
    {
      std::string code;
      code.assign("return set.").append(set_name_v<TestType>).append(".").append(toUpper(it.second));
      REQUIRE(luaL_dostring(L, code.c_str()) == LUA_OK);

      Lua::Set<TestType>::push(L, it.first);

      REQUIRE(lua_rawequal(L, -1, -2) == 1);
    }

    lua_close(L);
  }

  {
    INFO("add")

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, firstKey);
    Lua::Set<TestType>::push(L, lastKey);
    REQUIRE(protect<lua_arith>(L, LUA_OPADD));

    Lua::Set<TestType>::push(L, firstKey + lastKey);

    REQUIRE(lua_rawequal(L, -1, -2) == 1);

    lua_close(L);
  }

  {
    INFO("subtract")

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, mask);
    Lua::Set<TestType>::push(L, lastKey);
    REQUIRE(protect<lua_arith>(L, LUA_OPSUB));

    Lua::Set<TestType>::push(L, mask - lastKey);

    REQUIRE(lua_rawequal(L, -1, -2) == 1);

    lua_close(L);
  }

  {
    INFO("multiply")

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, mask);
    Lua::Set<TestType>::push(L, lastKey);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPMUL));

    lua_close(L);
  }

  {
    INFO("modulo")

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, mask);
    Lua::Set<TestType>::push(L, lastKey);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPMOD));

    lua_close(L);
  }

  {
    INFO("power")

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, mask);
    lua_pushnumber(L, 2.);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPPOW));

    lua_close(L);
  }

  {
    INFO("divide")

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, mask);
    Lua::Set<TestType>::push(L, lastKey);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPDIV));

    lua_close(L);
  }

  {
    INFO("divide (integer)")

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, mask);
    Lua::Set<TestType>::push(L, lastKey);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPIDIV));

    lua_close(L);
  }

  {
    INFO("binary and")

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, firstKey);
    Lua::Set<TestType>::push(L, lastKey);
    REQUIRE(protect<lua_arith>(L, LUA_OPBAND));

    Lua::Set<TestType>::push(L, firstKey & lastKey);

    REQUIRE(lua_rawequal(L, -1, -2) == 1);

    lua_close(L);
  }

  {
    INFO("binary or")

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, firstKey);
    Lua::Set<TestType>::push(L, lastKey);
    REQUIRE(protect<lua_arith>(L, LUA_OPBOR));

    Lua::Set<TestType>::push(L, firstKey | lastKey);

    REQUIRE(lua_rawequal(L, -1, -2) == 1);

    lua_close(L);
  }

  {
    INFO("binary xor")

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, firstKey);
    Lua::Set<TestType>::push(L, lastKey);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPBXOR));

    lua_close(L);
  }

  {
    INFO("shift left")

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, firstKey);
    lua_pushinteger(L, 1);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPSHL));

    lua_close(L);
  }

  {
    INFO("shift right")

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, firstKey);
    lua_pushinteger(L, 1);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPSHR));

    lua_close(L);
  }

  {
    INFO("unary minus")

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, firstKey);
    REQUIRE_FALSE(protect<lua_arith>(L, LUA_OPUNM));

    lua_close(L);
  }

  {
    INFO("binary not")

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, firstKey);
    REQUIRE(protect<lua_arith>(L, LUA_OPBNOT));

    Lua::Set<TestType>::push(L, ~firstKey);

    REQUIRE(lua_rawequal(L, -1, -2) == 1);

    lua_close(L);
  }
}
