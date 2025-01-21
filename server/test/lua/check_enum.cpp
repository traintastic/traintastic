/**
 * server/test/lua/check_enum.cpp
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
#include "../../src/lua/check.hpp"
#include "protect.hpp"
#include "../../src/lua/enums.hpp"
#include <type_traits>

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

template<class T>
static lua_State* createState()
{
  lua_State* L = newStateWithProtect();

  Lua::Enum<T>::registerType(L);
  Lua::Enum<typename other_enum_type<T>::type>::registerType(L);

  return L;
}

TEMPLATE_TEST_CASE("Lua::check<>", "[lua][lua-check]", LUA_ENUMS)
{
  using OtherEnumType = typename other_enum_type<TestType>::type;

  const TestType lastValue = EnumValues<TestType>::value.rbegin()->first;

  {
    INFO("nil");

    lua_State* L = createState<TestType>();

    lua_pushnil(L);
    TestType r;
    REQUIRE_FALSE(protect<Lua::check<TestType>>(r, L, -1));

    closeStateWithProtect(L);
  }

  {
    INFO("false");

    lua_State* L = createState<TestType>();

    lua_pushboolean(L, false);
    TestType r;
    REQUIRE_FALSE(protect<Lua::check<TestType>>(r, L, -1));

    closeStateWithProtect(L);
  }

  {
    INFO("true");

    lua_State* L = createState<TestType>();

    lua_pushboolean(L, true);
    TestType r;
    REQUIRE_FALSE(protect<Lua::check<TestType>>(r, L, -1));

    closeStateWithProtect(L);
  }

  {
    INFO("enum");

    lua_State* L = createState<TestType>();

    Lua::Enum<TestType>::push(L, lastValue);
    TestType r;
    REQUIRE(protect<Lua::check<TestType>>(r, L, -1));
    REQUIRE(r == lastValue);

    closeStateWithProtect(L);
  }

  {
    INFO("other enum");

    lua_State* L = createState<TestType>();

    Lua::Enum<OtherEnumType>::push(L, EnumValues<OtherEnumType>::value.rbegin()->first);
    TestType r;
    REQUIRE_FALSE(protect<Lua::check<TestType>>(r, L, -1));

    closeStateWithProtect(L);
  }

  {
    INFO("123");

    lua_State* L = createState<TestType>();

    lua_pushinteger(L, 123);
    TestType r;
    REQUIRE_FALSE(protect<Lua::check<TestType>>(r, L, -1));

    closeStateWithProtect(L);
  }

  {
    INFO("0.5");

    lua_State* L = createState<TestType>();

    lua_pushnumber(L, 0.5);
    TestType r;
    REQUIRE_FALSE(protect<Lua::check<TestType>>(r, L, -1));

    closeStateWithProtect(L);
  }

  {
    INFO("\"test\"");

    lua_State* L = createState<TestType>();

    lua_pushliteral(L, "test");
    TestType r;
    REQUIRE_FALSE(protect<Lua::check<TestType>>(r, L, -1));

    closeStateWithProtect(L);
  }

  {
    INFO("table");

    lua_State* L = createState<TestType>();

    lua_newtable(L);
    TestType r;
    REQUIRE_FALSE(protect<Lua::check<TestType>>(r, L, -1));

    closeStateWithProtect(L);
  }

  {
    INFO("userdata");

    lua_State* L = createState<TestType>();

    lua_newuserdata(L, 0);
    TestType r;
    REQUIRE_FALSE(protect<Lua::check<TestType>>(r, L, -1));

    closeStateWithProtect(L);
  }

  {
    INFO("lightuserdata");

    lua_State* L = createState<TestType>();

    lua_pushlightuserdata(L, nullptr);
    TestType r;
    REQUIRE_FALSE(protect<Lua::check<TestType>>(r, L, -1));

    closeStateWithProtect(L);
  }
}
