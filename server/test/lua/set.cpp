#include <catch2/catch.hpp>
#include "panicToException.hpp"
#include "../../src/lua/set.hpp"
#include "../../src/utils/toupper.hpp"

// Sets:
#include "../../src/set/worldstate.hpp"

template<class T>
static lua_State* createState()
{
  lua_State* L = luaL_newstate();

  lua_atpanic(L, panicToException);

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
    lua_arith(L, LUA_OPADD);

    Lua::Set<TestType>::push(L, firstKey + lastKey);

    REQUIRE(lua_rawequal(L, -1, -2) == 1);

    lua_close(L);
  }

  {
    INFO("subtract")

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, mask);
    Lua::Set<TestType>::push(L, lastKey);
    lua_arith(L, LUA_OPSUB);

    Lua::Set<TestType>::push(L, mask - lastKey);

    REQUIRE(lua_rawequal(L, -1, -2) == 1);

    lua_close(L);
  }

  {
    INFO("multiply")

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, mask);
    Lua::Set<TestType>::push(L, lastKey);
    REQUIRE_THROWS_AS(lua_arith(L, LUA_OPMUL), LuaPanicException);

    lua_close(L);
  }

  {
    INFO("modulo")

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, mask);
    Lua::Set<TestType>::push(L, lastKey);
    REQUIRE_THROWS_AS(lua_arith(L, LUA_OPMOD), LuaPanicException);

    lua_close(L);
  }

  {
    INFO("power")

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, mask);
    lua_pushnumber(L, 2.);
    REQUIRE_THROWS_AS(lua_arith(L, LUA_OPPOW), LuaPanicException);

    lua_close(L);
  }

  {
    INFO("divide")

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, mask);
    Lua::Set<TestType>::push(L, lastKey);
    REQUIRE_THROWS_AS(lua_arith(L, LUA_OPDIV), LuaPanicException);

    lua_close(L);
  }

  {
    INFO("divide (integer)")

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, mask);
    Lua::Set<TestType>::push(L, lastKey);
    REQUIRE_THROWS_AS(lua_arith(L, LUA_OPIDIV), LuaPanicException);

    lua_close(L);
  }

  {
    INFO("binary and")

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, firstKey);
    Lua::Set<TestType>::push(L, lastKey);
    lua_arith(L, LUA_OPBAND);

    Lua::Set<TestType>::push(L, firstKey & lastKey);

    REQUIRE(lua_rawequal(L, -1, -2) == 1);

    lua_close(L);
  }

  {
    INFO("binary or")

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, firstKey);
    Lua::Set<TestType>::push(L, lastKey);
    lua_arith(L, LUA_OPBOR);

    Lua::Set<TestType>::push(L, firstKey | lastKey);

    REQUIRE(lua_rawequal(L, -1, -2) == 1);

    lua_close(L);
  }

  {
    INFO("binary xor")

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, firstKey);
    Lua::Set<TestType>::push(L, lastKey);
    REQUIRE_THROWS_AS(lua_arith(L, LUA_OPBXOR), LuaPanicException);

    lua_close(L);
  }

  {
    INFO("shift left")

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, firstKey);
    lua_pushinteger(L, 1);
    REQUIRE_THROWS_AS(lua_arith(L, LUA_OPSHL), LuaPanicException);

    lua_close(L);
  }

  {
    INFO("shift right")

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, firstKey);
    lua_pushinteger(L, 1);
    REQUIRE_THROWS_AS(lua_arith(L, LUA_OPSHR), LuaPanicException);

    lua_close(L);
  }

  {
    INFO("unary minus")

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, firstKey);
    REQUIRE_THROWS_AS(lua_arith(L, LUA_OPUNM), LuaPanicException);

    lua_close(L);
  }

  {
    INFO("binary not")

    lua_State* L = createState<TestType>();

    Lua::Set<TestType>::push(L, firstKey);
    lua_arith(L, LUA_OPBNOT);

    Lua::Set<TestType>::push(L, ~firstKey);

    REQUIRE(lua_rawequal(L, -1, -2) == 1);

    lua_close(L);
  }
}
