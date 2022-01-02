/**
 * server/test/lua/getclass.cpp
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

TEST_CASE("Lua get_class", "[lua][lua-get_class]")
{
  static const char* get_class = "get_class";
  static const char* WORLD = "WORLD";
  static const char* DECODER = "DECODER";

  lua_State* L = luaL_newstate();

  // add in_instance function
  lua_pushcfunction(L, Lua::Class::getClass);
  lua_setglobal(L, get_class);

  // add classes
  lua_newtable(L);
  Lua::Class::registerValues(L);

  // register object type
  Lua::Object::registerType(L);

  // nil
  lua_getglobal(L, get_class);
  lua_pushnil(L);
  REQUIRE(lua_pcall(L, 1, 1, 0) == LUA_ERRRUN);
  lua_pop(L, 1);

  // boolean: false
  lua_getglobal(L, get_class);
  lua_pushboolean(L, 0);
  REQUIRE(lua_pcall(L, 1, 1, 0) == LUA_ERRRUN);
  lua_pop(L, 1);

  // boolean: true
  lua_getglobal(L, get_class);
  lua_pushboolean(L, 1);
  REQUIRE(lua_pcall(L, 1, 1, 0) == LUA_ERRRUN);
  lua_pop(L, 1);

  // integer: 42
  lua_getglobal(L, get_class);
  lua_pushinteger(L, 42);
  REQUIRE(lua_pcall(L, 1, 1, 0) == LUA_ERRRUN);
  lua_pop(L, 1);

  // number: 3.14
  lua_getglobal(L, get_class);
  lua_pushnumber(L, 3.14);
  REQUIRE(lua_pcall(L, 1, 1, 0) == LUA_ERRRUN);
  lua_pop(L, 1);

  // string
  lua_getglobal(L, get_class);
  lua_pushstring(L, "traintastic");
  REQUIRE(lua_pcall(L, 1, 1, 0) == LUA_ERRRUN);
  lua_pop(L, 1);

  // table
  lua_getglobal(L, get_class);
  lua_newtable(L);
  REQUIRE(lua_pcall(L, 1, 1, 0) == LUA_ERRRUN);
  lua_pop(L, 1);

  // userdata
  lua_getglobal(L, get_class);
  lua_newuserdata(L, sizeof(void*));
  REQUIRE(lua_pcall(L, 1, 1, 0) == LUA_ERRRUN);
  lua_pop(L, 1);

  // World
  auto world = World::create();
  lua_getglobal(L, get_class);
  Lua::Object::push(L, world);
  REQUIRE(lua_pcall(L, 1, 1, 0) == LUA_OK);
  REQUIRE(lua_isuserdata(L, -1));
  lua_getfield(L, -2, WORLD);
  REQUIRE(lua_rawequal(L, -1, -2) == 1);
  lua_pop(L, 2);

  lua_getglobal(L, get_class);
  Lua::Object::push(L, world);
  REQUIRE(lua_pcall(L, 1, 1, 0) == LUA_OK);
  REQUIRE(lua_isuserdata(L, -1));
  lua_getfield(L, -2, DECODER);
  REQUIRE(lua_rawequal(L, -1, -2) == 0);
  lua_pop(L, 2);

  // Decoder
  auto decoder = world->decoders->add();
  lua_getglobal(L, get_class);
  Lua::Object::push(L, decoder);
  REQUIRE(lua_pcall(L, 1, 1, 0) == LUA_OK);
  REQUIRE(lua_isuserdata(L, -1));
  lua_getfield(L, -2, WORLD);
  REQUIRE(lua_rawequal(L, -1, -2) == 0);
  lua_pop(L, 2);

  lua_getglobal(L, get_class);
  Lua::Object::push(L, decoder);
  REQUIRE(lua_pcall(L, 1, 1, 0) == LUA_OK);
  REQUIRE(lua_isuserdata(L, -1));
  lua_getfield(L, -2, DECODER);
  REQUIRE(lua_rawequal(L, -1, -2) == 1);
  lua_pop(L, 2);

  lua_close(L);
}
