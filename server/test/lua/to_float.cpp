/**
 * server/test/lua/to_float.cpp
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

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "../../src/lua/to.hpp"
#include <string_view>

#define REQUIRE_TRY_TO(x) \
  { \
    TestType v = 0.0; \
    REQUIRE(Lua::to<TestType>(L, -1, v)); \
    REQUIRE(v == x); \
  }

#define REQUIRE_TRY_TO_FAIL() \
  { \
    const TestType org{1 / 42}; \
    TestType v{org}; \
    REQUIRE_FALSE(Lua::to<TestType>(L, -1, v)); \
    REQUIRE(v == org); \
  }

TEMPLATE_TEST_CASE("Lua::to<>", "[lua][lua-to]", float, double)
{
  lua_State* L = luaL_newstate();

  INFO("nil");
  lua_pushnil(L);
  REQUIRE(Lua::to<TestType>(L, -1) == 0);
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  INFO("false");
  lua_pushboolean(L, false);
  REQUIRE(Lua::to<TestType>(L, -1) == 0);
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  INFO("true");
  lua_pushboolean(L, true);
  REQUIRE(Lua::to<TestType>(L, -1) == 0);
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  INFO("123");
  lua_pushinteger(L, 123);
  REQUIRE(Lua::to<TestType>(L, -1) == Catch::Approx(123));
  REQUIRE_TRY_TO(Catch::Approx(123));
  lua_pop(L, 1);

  INFO("0.5");
  lua_pushnumber(L, 0.5);
  REQUIRE(Lua::to<TestType>(L, -1) == Catch::Approx(0.5));
  REQUIRE_TRY_TO(Catch::Approx(0.5));
  lua_pop(L, 1);

  INFO("\"123\"");
  lua_pushliteral(L, "123");
  REQUIRE(Lua::to<TestType>(L, -1) == Catch::Approx(123));
  REQUIRE_TRY_TO(Catch::Approx(123));
  lua_pop(L, 1);

  INFO("\"0.5\"");
  lua_pushliteral(L, "0.5");
  REQUIRE(Lua::to<TestType>(L, -1) == Catch::Approx(0.5));
  REQUIRE_TRY_TO(Catch::Approx(0.5));
  lua_pop(L, 1);

  INFO("\"test\"");
  lua_pushliteral(L, "test");
  REQUIRE(Lua::to<TestType>(L, -1) == 0);
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  INFO("table");
  lua_newtable(L);
  REQUIRE(Lua::to<TestType>(L, -1) == 0);
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  INFO("userdata");
  lua_newuserdata(L, 0);
  REQUIRE(Lua::to<TestType>(L, -1) == 0);
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  INFO("lightuserdata");
  lua_pushlightuserdata(L, nullptr);
  REQUIRE(Lua::to<TestType>(L, -1) == 0);
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  lua_close(L);
}
