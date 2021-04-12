#include <catch2/catch.hpp>
#include "panicToException.hpp"
#include "../../src/lua/enum.hpp"
#include "../../src/utils/toupper.hpp"

// Enums:
#include "../../src/enum/decoderprotocol.hpp"
#include "../../src/enum/direction.hpp"
#include "../../src/enum/worldevent.hpp"

template<class T>
static lua_State* createState()
{
  lua_State* L = luaL_newstate();

  lua_atpanic(L, panicToException);

  Lua::Enum<T>::registerType(L);

  lua_createtable(L, 0, 1);
  Lua::Enum<T>::registerValues(L);
  lua_setglobal(L, "enum");

  return L;
}

TEMPLATE_TEST_CASE("Lua::Enum<>", "[lua][lua-enum]", DecoderProtocol, Direction, WorldEvent)
{
  const TestType firstKey = EnumValues<TestType>::value.begin()->first;
  const TestType lastKey = EnumValues<TestType>::value.rbegin()->first;

  {
    INFO("single value")

    lua_State* L = createState<TestType>();

    for(auto& it : EnumValues<TestType>::value)
    {
      std::string code;
      code.assign("return enum.").append(EnumName<TestType>::value).append(".").append(toUpper(it.second));
      REQUIRE(luaL_dostring(L, code.c_str()) == LUA_OK);

      Lua::Enum<TestType>::push(L, it.first);

      REQUIRE(lua_rawequal(L, -1, -2) == 1);
    }

    lua_close(L);
  }

  {
    INFO("tostring")

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

    lua_close(L);
  }

  {
    INFO("add")

    lua_State* L = createState<TestType>();

    Lua::Enum<TestType>::push(L, firstKey);
    Lua::Enum<TestType>::push(L, lastKey);
    REQUIRE_THROWS_AS(lua_arith(L, LUA_OPADD), LuaPanicException);

    lua_close(L);
  }

  {
    INFO("subtract")

    lua_State* L = createState<TestType>();

    Lua::Enum<TestType>::push(L, firstKey);
    Lua::Enum<TestType>::push(L, lastKey);
    REQUIRE_THROWS_AS(lua_arith(L, LUA_OPSUB), LuaPanicException);

    lua_close(L);
  }

  {
    INFO("multiply")

    lua_State* L = createState<TestType>();

    Lua::Enum<TestType>::push(L, firstKey);
    Lua::Enum<TestType>::push(L, lastKey);
    REQUIRE_THROWS_AS(lua_arith(L, LUA_OPMUL), LuaPanicException);

    lua_close(L);
  }

  {
    INFO("modulo")

    lua_State* L = createState<TestType>();

    Lua::Enum<TestType>::push(L, firstKey);
    Lua::Enum<TestType>::push(L, lastKey);
    REQUIRE_THROWS_AS(lua_arith(L, LUA_OPMOD), LuaPanicException);

    lua_close(L);
  }

  {
    INFO("power")

    lua_State* L = createState<TestType>();

    Lua::Enum<TestType>::push(L, firstKey);
    lua_pushnumber(L, 2.);
    REQUIRE_THROWS_AS(lua_arith(L, LUA_OPPOW), LuaPanicException);

    lua_close(L);
  }

  {
    INFO("divide")

    lua_State* L = createState<TestType>();

    Lua::Enum<TestType>::push(L, firstKey);
    Lua::Enum<TestType>::push(L, lastKey);
    REQUIRE_THROWS_AS(lua_arith(L, LUA_OPDIV), LuaPanicException);

    lua_close(L);
  }

  {
    INFO("divide (integer)")

    lua_State* L = createState<TestType>();

    Lua::Enum<TestType>::push(L, firstKey);
    Lua::Enum<TestType>::push(L, lastKey);
    REQUIRE_THROWS_AS(lua_arith(L, LUA_OPIDIV), LuaPanicException);

    lua_close(L);
  }

  {
    INFO("binary and")

    lua_State* L = createState<TestType>();

    Lua::Enum<TestType>::push(L, firstKey);
    Lua::Enum<TestType>::push(L, lastKey);
    REQUIRE_THROWS_AS(lua_arith(L, LUA_OPBAND), LuaPanicException);

    lua_close(L);
  }

  {
    INFO("binary or")

    lua_State* L = createState<TestType>();

    Lua::Enum<TestType>::push(L, firstKey);
    Lua::Enum<TestType>::push(L, lastKey);
    REQUIRE_THROWS_AS(lua_arith(L, LUA_OPBOR), LuaPanicException);

    lua_close(L);
  }

  {
    INFO("binary xor")

    lua_State* L = createState<TestType>();

    Lua::Enum<TestType>::push(L, firstKey);
    Lua::Enum<TestType>::push(L, lastKey);
    REQUIRE_THROWS_AS(lua_arith(L, LUA_OPBXOR), LuaPanicException);

    lua_close(L);
  }

  {
    INFO("shift left")

    lua_State* L = createState<TestType>();

    Lua::Enum<TestType>::push(L, firstKey);
    lua_pushinteger(L, 1);
    REQUIRE_THROWS_AS(lua_arith(L, LUA_OPSHL), LuaPanicException);

    lua_close(L);
  }

  {
    INFO("shift right")

    lua_State* L = createState<TestType>();

    Lua::Enum<TestType>::push(L, firstKey);
    lua_pushinteger(L, 1);
    REQUIRE_THROWS_AS(lua_arith(L, LUA_OPSHR), LuaPanicException);

    lua_close(L);
  }

  {
    INFO("unary minus")

    lua_State* L = createState<TestType>();

    Lua::Enum<TestType>::push(L, firstKey);
    REQUIRE_THROWS_AS(lua_arith(L, LUA_OPUNM), LuaPanicException);

    lua_close(L);
  }

  {
    INFO("binary not")

    lua_State* L = createState<TestType>();

    Lua::Enum<TestType>::push(L, firstKey);
    REQUIRE_THROWS_AS(lua_arith(L, LUA_OPBNOT), LuaPanicException);

    lua_close(L);
  }
}
