/**
 * server/test/lua/check_int.cpp
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
#include <string_view>

TEMPLATE_TEST_CASE("Lua::check<>", "[lua][lua-check]", int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t)
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
    REQUIRE(r == 123);

    closeStateWithProtect(L);
  }

  {
    INFO("1000");

    lua_State* L = newStateWithProtect();

    lua_pushinteger(L, 1000);
    TestType r;
    if constexpr(std::numeric_limits<TestType>::max() < 1000)
    {
      REQUIRE_FALSE(protect<Lua::check<TestType>>(r, L, -1));
    }
    else
    {
      REQUIRE(protect<Lua::check<TestType>>(r, L, -1));
      REQUIRE(r == 1000);
    }

    closeStateWithProtect(L);
  }

  {
    INFO("-1000");

    lua_State* L = newStateWithProtect();

    lua_pushinteger(L, -1000);
    TestType r;
    if constexpr(std::is_unsigned_v<TestType>)
    {
      REQUIRE_FALSE(protect<Lua::check<TestType>>(r, L, -1));
    }
    else if(std::numeric_limits<TestType>::min() > -1000)
    {
      REQUIRE_FALSE(protect<Lua::check<TestType>>(r, L, -1));
    }
    else
    {
      REQUIRE(protect<Lua::check<TestType>>(r, L, -1));
      REQUIRE(r == -1000);
    }

    closeStateWithProtect(L);
  }

  {
    INFO("4.2");

    lua_State* L = newStateWithProtect();

    lua_pushnumber(L, 4.2);
    TestType r;
    REQUIRE_FALSE(protect<Lua::check<TestType>>(r, L, -1));

    closeStateWithProtect(L);
  }

  {
    INFO("\"123\"");

    lua_State* L = newStateWithProtect();

    lua_pushliteral(L, "123");
    TestType r;
    REQUIRE(protect<Lua::check<TestType>>(r, L, -1));
    REQUIRE(r == 123);

    closeStateWithProtect(L);
  }

  {
    INFO("\"1000\"");

    lua_State* L = newStateWithProtect();

    lua_pushliteral(L, "1000");
    TestType r;
    if(std::numeric_limits<TestType>::max() < 1000)
    {
      REQUIRE_FALSE(protect<Lua::check<TestType>>(r, L, -1));
    }
    else
    {
      REQUIRE(protect<Lua::check<TestType>>(r, L, -1));
      REQUIRE(r == 1000);
    }

    closeStateWithProtect(L);
  }

  {
    INFO("\"-1000\"");

    lua_State* L = newStateWithProtect();

    lua_pushliteral(L, "-1000");
    TestType r;
    if constexpr(std::is_unsigned_v<TestType>)
    {
      REQUIRE_FALSE(protect<Lua::check<TestType>>(r, L, -1));
    }
    else if(std::numeric_limits<TestType>::min() > -1000)
    {
      REQUIRE_FALSE(protect<Lua::check<TestType>>(r, L, -1));
    }
    else
    {
      REQUIRE(protect<Lua::check<TestType>>(r, L, -1));
      REQUIRE(r == -1000);
    }

    closeStateWithProtect(L);
  }

  {
    INFO("\"4.2\"");

    lua_State* L = newStateWithProtect();

    lua_pushliteral(L, "4.2");
    TestType r;
    REQUIRE_FALSE(protect<Lua::check<TestType>>(r, L, -1));

    closeStateWithProtect(L);
  }

  {
    INFO("\"test\"");

    lua_State* L = newStateWithProtect();

    lua_pushliteral(L, "test");
    TestType r;
    REQUIRE_FALSE(protect<Lua::check<TestType>>(r, L, -1));

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
