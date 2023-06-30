/**
 * server/test/lua/push.cpp
 *
 * This file is part of the traintastic test suite.
 *
 * Copyright (C) 2021-2023 Reinder Feenstra
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
#include "../../src/lua/push.hpp"

// Enums:
#include "../../src/lua/enums.hpp"

// Sets:
#include "../../src/lua/sets.hpp"

// Objects:
#include "../../src/world/world.hpp"

TEMPLATE_TEST_CASE("Lua::push<>", "[lua][lua-push]", std::nullptr_t)
{
  lua_State* L = luaL_newstate();
  const int top = lua_gettop(L);

  Lua::push(L, nullptr);
  REQUIRE(lua_type(L, -1) == LUA_TNIL);
  REQUIRE(lua_gettop(L) == top + 1);

  lua_close(L);
}

TEMPLATE_TEST_CASE("Lua::push<>", "[lua][lua-push]", bool)
{
  lua_State* L = luaL_newstate();
  const int top = lua_gettop(L);

  Lua::push(L, false);
  REQUIRE(lua_type(L, -1) == LUA_TBOOLEAN);
  REQUIRE(lua_gettop(L) == top + 1);

  Lua::push(L, true);
  REQUIRE(lua_type(L, -1) == LUA_TBOOLEAN);
  REQUIRE(lua_gettop(L) == top + 2);

  lua_close(L);
}

TEMPLATE_TEST_CASE("Lua::push<>", "[lua][lua-push]", int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t)
{
  lua_State* L = luaL_newstate();
  const int top = lua_gettop(L);

  Lua::push(L, std::numeric_limits<TestType>::min());
  REQUIRE(lua_type(L, -1) == LUA_TNUMBER);
  REQUIRE(lua_gettop(L) == top + 1);

  Lua::push(L, std::numeric_limits<TestType>::max());
  REQUIRE(lua_type(L, -1) == LUA_TNUMBER);
  REQUIRE(lua_gettop(L) == top + 2);

  lua_close(L);
}

TEMPLATE_TEST_CASE("Lua::push<>", "[lua][lua-push]", float, double)
{
  lua_State* L = luaL_newstate();
  const int top = lua_gettop(L);

  Lua::push(L, -std::numeric_limits<TestType>::max());
  REQUIRE(lua_type(L, -1) == LUA_TNUMBER);
  REQUIRE(lua_gettop(L) == top + 1);

  Lua::push(L, std::numeric_limits<TestType>::max());
  REQUIRE(lua_type(L, -1) == LUA_TNUMBER);
  REQUIRE(lua_gettop(L) == top + 2);

  Lua::push(L, std::numeric_limits<TestType>::quiet_NaN());
  REQUIRE(lua_type(L, -1) == LUA_TNUMBER);
  REQUIRE(lua_gettop(L) == top + 3);

  Lua::push(L, std::numeric_limits<TestType>::infinity());
  REQUIRE(lua_type(L, -1) == LUA_TNUMBER);
  REQUIRE(lua_gettop(L) == top + 4);

  lua_close(L);
}

TEMPLATE_TEST_CASE("Lua::push<>", "[lua][lua-push]", std::string, std::string_view)
{
  lua_State* L = luaL_newstate();
  const int top = lua_gettop(L);

  Lua::push(L, TestType{"some text"});
  REQUIRE(lua_type(L, -1) == LUA_TSTRING);
  REQUIRE(lua_gettop(L) == top + 1);

  Lua::push(L, TestType{""});
  REQUIRE(lua_type(L, -1) == LUA_TSTRING);
  REQUIRE(lua_gettop(L) == top + 2);

  lua_close(L);
}

TEMPLATE_TEST_CASE("Lua::push<>", "[lua][lua-push]", LUA_ENUMS)
{
  lua_State* L = luaL_newstate();
  const int top = lua_gettop(L);

  Lua::Enum<TestType>::registerType(L);

  Lua::push(L, EnumValues<TestType>::value.begin()->first);
  REQUIRE(lua_type(L, -1) == LUA_TUSERDATA);
  REQUIRE(lua_gettop(L) == top + 1);

  lua_close(L);
}

TEMPLATE_TEST_CASE("Lua::push<>", "[lua][lua-push]", LUA_SETS)
{
  lua_State* L = luaL_newstate();
  const int top = lua_gettop(L);

  Lua::Set<TestType>::registerType(L);

  Lua::push(L, set_values_v<TestType>.begin()->first);
  REQUIRE(lua_type(L, -1) == LUA_TUSERDATA);
  REQUIRE(lua_gettop(L) == top + 1);

  lua_close(L);
}

TEMPLATE_TEST_CASE("Lua::push<>", "[lua][lua-push]", World)
{
  lua_State* L = luaL_newstate();
  const int top = lua_gettop(L);

  Lua::Object::registerTypes(L);

  Lua::push(L, World::create());
  REQUIRE(lua_type(L, -1) == LUA_TUSERDATA);
  REQUIRE(lua_gettop(L) == top + 1);

  lua_close(L);
}
