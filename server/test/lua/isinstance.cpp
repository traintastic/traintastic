/**
 * server/test/lua/isinstance.cpp
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

#include <catch2/catch.hpp>
#include "../../src/lua/class.hpp"
#include "../../src/lua/object.hpp"
#include "../../src/world/world.hpp"

TEST_CASE("Lua is_instance", "[lua][lua-isinstance]")
{
  static const char* is_instance = "is_instance";
  static const char* WORLD = "WORLD";
  static const char* DECODER = "DECODER";

  lua_State* L = luaL_newstate();

  // add in_instance function
  lua_pushcfunction(L, Lua::Class::isInstance);
  lua_setglobal(L, is_instance);

  // add classes
  lua_newtable(L);
  Lua::Class::registerValues(L);

  // register object type
  Lua::Object::registerType(L);

  // nil
  lua_getglobal(L, is_instance);
  lua_pushnil(L);
  lua_getfield(L, -3, WORLD);
  REQUIRE(lua_pcall(L, 2, 1, 0) == LUA_OK);
  REQUIRE(lua_isboolean(L, -1));
  REQUIRE_FALSE(lua_toboolean(L, -1));
  lua_pop(L, 1);

  // boolean: false
  lua_getglobal(L, is_instance);
  lua_pushboolean(L, 0);
  lua_getfield(L, -3, WORLD);
  REQUIRE(lua_pcall(L, 2, 1, 0) == LUA_OK);
  REQUIRE(lua_isboolean(L, -1));
  REQUIRE_FALSE(lua_toboolean(L, -1));
  lua_pop(L, 1);

  // boolean: true
  lua_getglobal(L, is_instance);
  lua_pushboolean(L, 1);
  lua_getfield(L, -3, WORLD);
  REQUIRE(lua_pcall(L, 2, 1, 0) == LUA_OK);
  REQUIRE(lua_isboolean(L, -1));
  REQUIRE_FALSE(lua_toboolean(L, -1));
  lua_pop(L, 1);

  // integer: 42
  lua_getglobal(L, is_instance);
  lua_pushinteger(L, 42);
  lua_getfield(L, -3, WORLD);
  REQUIRE(lua_pcall(L, 2, 1, 0) == LUA_OK);
  REQUIRE(lua_isboolean(L, -1));
  REQUIRE_FALSE(lua_toboolean(L, -1));
  lua_pop(L, 1);

  // number: 3.14
  lua_getglobal(L, is_instance);
  lua_pushnumber(L, 3.14);
  lua_getfield(L, -3, WORLD);
  REQUIRE(lua_pcall(L, 2, 1, 0) == LUA_OK);
  REQUIRE(lua_isboolean(L, -1));
  REQUIRE_FALSE(lua_toboolean(L, -1));
  lua_pop(L, 1);

  // string
  lua_getglobal(L, is_instance);
  lua_pushstring(L, "traintastic");
  lua_getfield(L, -3, WORLD);
  REQUIRE(lua_pcall(L, 2, 1, 0) == LUA_OK);
  REQUIRE(lua_isboolean(L, -1));
  REQUIRE_FALSE(lua_toboolean(L, -1));
  lua_pop(L, 1);

  // table
  lua_getglobal(L, is_instance);
  lua_newtable(L);
  lua_getfield(L, -3, WORLD);
  REQUIRE(lua_pcall(L, 2, 1, 0) == LUA_OK);
  REQUIRE(lua_isboolean(L, -1));
  REQUIRE_FALSE(lua_toboolean(L, -1));
  lua_pop(L, 1);

  // userdata
  lua_getglobal(L, is_instance);
  lua_newuserdata(L, sizeof(void*));
  lua_getfield(L, -3, WORLD);
  REQUIRE(lua_pcall(L, 2, 1, 0) == LUA_OK);
  REQUIRE(lua_isboolean(L, -1));
  REQUIRE_FALSE(lua_toboolean(L, -1));
  lua_pop(L, 1);

  // World
  auto world = World::create();
  lua_getglobal(L, is_instance);
  Lua::Object::push(L, world);
  lua_getfield(L, -3, WORLD);
  REQUIRE(lua_pcall(L, 2, 1, 0) == LUA_OK);
  REQUIRE(lua_isboolean(L, -1));
  REQUIRE(lua_toboolean(L, -1));
  lua_pop(L, 1);

  lua_getglobal(L, is_instance);
  Lua::Object::push(L, world);
  lua_getfield(L, -3, DECODER);
  REQUIRE(lua_pcall(L, 2, 1, 0) == LUA_OK);
  REQUIRE(lua_isboolean(L, -1));
  REQUIRE_FALSE(lua_toboolean(L, -1));
  lua_pop(L, 1);

  // Decoder
  auto decoder = world->decoders->add();
  lua_getglobal(L, is_instance);
  Lua::Object::push(L, decoder);
  lua_getfield(L, -3, WORLD);
  REQUIRE(lua_pcall(L, 2, 1, 0) == LUA_OK);
  REQUIRE(lua_isboolean(L, -1));
  REQUIRE_FALSE(lua_toboolean(L, -1));
  lua_pop(L, 1);

  lua_getglobal(L, is_instance);
  Lua::Object::push(L, decoder);
  lua_getfield(L, -3, DECODER);
  REQUIRE(lua_pcall(L, 2, 1, 0) == LUA_OK);
  REQUIRE(lua_isboolean(L, -1));
  REQUIRE(lua_toboolean(L, -1));
  lua_pop(L, 1);

  lua_close(L);
}
