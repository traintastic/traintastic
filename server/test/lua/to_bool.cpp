#include <catch2/catch.hpp>
#include "../../src/lua/to.hpp"
#include <string_view>

#define REQUIRE_TRY_TO_FAIL() \
  { \
    TestType v = false; \
    REQUIRE_FALSE(Lua::to<TestType>(L, -1, v)); \
    REQUIRE_FALSE(v); \
    v = true; \
    REQUIRE_FALSE(Lua::to<TestType>(L, -1, v)); \
    REQUIRE(v); \
  }

TEMPLATE_TEST_CASE("Lua::to<>", "[lua][lua-to]", bool)
{
  lua_State* L = luaL_newstate();

  INFO("nil")
  lua_pushnil(L);
  REQUIRE_FALSE(Lua::to<TestType>(L, -1));
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  INFO("false")
  lua_pushboolean(L, false);
  REQUIRE_FALSE(Lua::to<TestType>(L, -1));
  {
    bool v = true;
    REQUIRE(Lua::to<TestType>(L, -1, v));
    REQUIRE_FALSE(v);
  }
  lua_pop(L, 1);

  INFO("true")
  lua_pushboolean(L, true);
  REQUIRE(Lua::to<TestType>(L, -1));
  {
    bool v = false;
    REQUIRE(Lua::to<TestType>(L, -1, v));
    REQUIRE(v);
  }
  lua_pop(L, 1);

  INFO("123")
  lua_pushinteger(L, 123);
  REQUIRE_FALSE(Lua::to<TestType>(L, -1));
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  INFO("0.5")
  REQUIRE_FALSE(Lua::to<TestType>(L, -1));
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  INFO("\"test\"")
  lua_pushliteral(L, "test");
  REQUIRE_FALSE(Lua::to<TestType>(L, -1));
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  INFO("table")
  lua_newtable(L);
  REQUIRE_FALSE(Lua::to<TestType>(L, -1));
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  INFO("userdata")
  lua_newuserdata(L, 0);
  REQUIRE_FALSE(Lua::to<TestType>(L, -1));
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  INFO("lightuserdata")
  lua_pushlightuserdata(L, nullptr);
  REQUIRE_FALSE(Lua::to<TestType>(L, -1));
  REQUIRE_TRY_TO_FAIL();
  lua_pop(L, 1);

  lua_close(L);
}
