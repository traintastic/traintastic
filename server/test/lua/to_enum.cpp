/**
 * server/test/lua/to_enum.cpp
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
#include "../../src/lua/enums.hpp"
#include "../../src/lua/to.hpp"
#include <string_view>
#include <type_traits>

#define REQUIRE_TRY_TO_FAIL() \
  { \
    TestType v = EnumValues<TestType>::value.begin()->first; \
    REQUIRE_FALSE(Lua::to<TestType>(L, -1, v)); \
    REQUIRE(v == EnumValues<TestType>::value.begin()->first); \
    v = EnumValues<TestType>::value.rbegin()->first; \
    REQUIRE_FALSE(Lua::to<TestType>(L, -1, v)); \
    REQUIRE(v == EnumValues<TestType>::value.rbegin()->first); \
  }

template<class T>
struct other_enum_type
{
  using type = WorldEvent;
};

template<>
struct other_enum_type<WorldEvent>
{
  using type = DecoderProtocol;
};

TEMPLATE_TEST_CASE("Lua::to<>", "[lua][lua-to]", LUA_ENUMS)
{
  using OtherEnumType = typename other_enum_type<TestType>::type;

  const TestType firstValue = EnumValues<TestType>::value.begin()->first;
  const TestType lastValue = EnumValues<TestType>::value.rbegin()->first;

  lua_State* L = luaL_newstate();

  INFO("nil");
  lua_pushnil(L);
  REQUIRE(Lua::to<TestType>(L, -1) == firstValue);
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  INFO("false");
  lua_pushboolean(L, false);
  REQUIRE(Lua::to<TestType>(L, -1) == firstValue);
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  INFO("true");
  lua_pushboolean(L, true);
  REQUIRE(Lua::to<TestType>(L, -1) == firstValue);
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  INFO("enum");
  Lua::Enum<TestType>::registerType(L);
  Lua::Enum<TestType>::push(L, lastValue);
  REQUIRE(Lua::to<TestType>(L, -1) == lastValue);
  {
    TestType v = firstValue;
    REQUIRE(Lua::to<TestType>(L, -1, v));
    REQUIRE(v == lastValue);
  }
  lua_pop(L, 1);

  INFO("other enum");
  Lua::Enum<OtherEnumType>::registerType(L);
  Lua::Enum<OtherEnumType>::push(L, EnumValues<OtherEnumType>::value.rbegin()->first);
  REQUIRE(Lua::to<TestType>(L, -1) == firstValue);
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  INFO("123");
  lua_pushinteger(L, 123);
  REQUIRE(Lua::to<TestType>(L, -1) == firstValue);
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  INFO("0.5");
  lua_pushnumber(L, 0.5);
  REQUIRE(Lua::to<TestType>(L, -1) == firstValue);
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  INFO("\"test\"");
  lua_pushliteral(L, "test");
  REQUIRE(Lua::to<TestType>(L, -1) == firstValue);
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  INFO("table");
  lua_newtable(L);
  REQUIRE(Lua::to<TestType>(L, -1) == firstValue);
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  INFO("userdata");
  lua_newuserdata(L, 0);
  REQUIRE(Lua::to<TestType>(L, -1) == firstValue);
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  INFO("lightuserdata");
  lua_pushlightuserdata(L, nullptr);
  REQUIRE(Lua::to<TestType>(L, -1) == firstValue);
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  lua_close(L);
}
