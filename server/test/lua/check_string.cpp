/**
 * server/test/lua/check_string.cpp
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
#include "protect.hpp"
#include "../../src/lua/check.hpp"

TEMPLATE_TEST_CASE("Lua::check<>", "[lua][lua-check]", std::string, std::string_view)
{
  {
    INFO("nil");

    lua_State* L = newStateWithProtect();

    lua_pushnil(L);
    TestType r;
    REQUIRE_FALSE(protect<Lua::check<TestType>>(r, L, -1));

    closeStateWithProtect(L);
  }

  {
    INFO("false");

    lua_State* L = newStateWithProtect();

    lua_pushboolean(L, false);
    TestType r;
    REQUIRE_FALSE(protect<Lua::check<TestType>>(r, L, -1));

    closeStateWithProtect(L);
  }

  {
    INFO("true");

    lua_State* L = newStateWithProtect();

    lua_pushboolean(L, true);
    TestType r;
    REQUIRE_FALSE(protect<Lua::check<TestType>>(r, L, -1));

    closeStateWithProtect(L);
  }

  {
    INFO("123");

    lua_State* L = newStateWithProtect();

    lua_pushinteger(L, 123);
    TestType r;
    REQUIRE(protect<Lua::check<TestType>>(r, L, -1));
    REQUIRE(r == "123");
    REQUIRE(lua_type(L, -1) == LUA_TSTRING);

    closeStateWithProtect(L);
  }

  {
    INFO("0.5");

    lua_State* L = newStateWithProtect();

    lua_pushnumber(L, 0.5);
    TestType r;
    REQUIRE(protect<Lua::check<TestType>>(r, L, -1));
    REQUIRE(r == "0.5");
    REQUIRE(lua_type(L, -1) == LUA_TSTRING);

    closeStateWithProtect(L);
  }

  {
    INFO("\"test\"");

    lua_State* L = newStateWithProtect();

    lua_pushliteral(L, "test");
    TestType r;
    REQUIRE(protect<Lua::check<TestType>>(r, L, -1));
    REQUIRE(r == "test");

    closeStateWithProtect(L);
  }

  {
    INFO("table");

    lua_State* L = newStateWithProtect();

    lua_newtable(L);
    TestType r;
    REQUIRE_FALSE(protect<Lua::check<TestType>>(r, L, -1));

    closeStateWithProtect(L);
  }

  {
    INFO("userdata");

    lua_State* L = newStateWithProtect();

    lua_newuserdata(L, 0);
    TestType r;
    REQUIRE_FALSE(protect<Lua::check<TestType>>(r, L, -1));

    closeStateWithProtect(L);
  }

  {
    INFO("lightuserdata");

    lua_State* L = newStateWithProtect();

    lua_pushlightuserdata(L, nullptr);
    TestType r;
    REQUIRE_FALSE(protect<Lua::check<TestType>>(r, L, -1));

    closeStateWithProtect(L);
  }
}
