#include <catch2/catch.hpp>
#include "../../src/lua/to.hpp"
#include <string_view>

#define REQUIRE_TRY_TO(x) \
  { \
    TestType v; \
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

  INFO("nil")
  lua_pushnil(L);
  REQUIRE(Lua::to<TestType>(L, -1) == 0);
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  INFO("false")
  lua_pushboolean(L, false);
  REQUIRE(Lua::to<TestType>(L, -1) == 0);
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  INFO("true")
  lua_pushboolean(L, true);
  REQUIRE(Lua::to<TestType>(L, -1) == 0);
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  INFO("123")
  lua_pushinteger(L, 123);
  REQUIRE(Lua::to<TestType>(L, -1) == Approx(123));
  REQUIRE_TRY_TO(Approx(123));
  lua_pop(L, 1);

  INFO("0.5")
  lua_pushnumber(L, 0.5);
  REQUIRE(Lua::to<TestType>(L, -1) == Approx(0.5));
  REQUIRE_TRY_TO(Approx(0.5));
  lua_pop(L, 1);

  INFO("\"123\"")
  lua_pushliteral(L, "123");
  REQUIRE(Lua::to<TestType>(L, -1) == Approx(123));
  REQUIRE_TRY_TO(Approx(123));
  lua_pop(L, 1);

  INFO("\"0.5\"")
  lua_pushliteral(L, "0.5");
  REQUIRE(Lua::to<TestType>(L, -1) == Approx(0.5));
  REQUIRE_TRY_TO(Approx(0.5));
  lua_pop(L, 1);

  INFO("\"test\"")
  lua_pushliteral(L, "test");
  REQUIRE(Lua::to<TestType>(L, -1) == 0);
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  INFO("table")
  lua_newtable(L);
  REQUIRE(Lua::to<TestType>(L, -1) == 0);
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  INFO("userdata")
  lua_newuserdata(L, 0);
  REQUIRE(Lua::to<TestType>(L, -1) == 0);
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  INFO("lightuserdata")
  lua_pushlightuserdata(L, nullptr);
  REQUIRE(Lua::to<TestType>(L, -1) == 0);
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  lua_close(L);
}
