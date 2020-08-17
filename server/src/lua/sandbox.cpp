/**
 * server/src/lua/sandbox.cpp - Lua sandbox
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

#include "sandbox.hpp"
#include "push.hpp"
#include "object.hpp"
#include "method.hpp"
#include "console.hpp"
#include <traintastic/utils/str.hpp>
#include <traintastic/codename.hpp>
#include "../world/world.hpp"
#include "../enum/decoderprotocol.hpp"
#include "../enum/direction.hpp"
#include "../enum/worldevent.hpp"
#include "../set/worldstate.hpp"

#define LUA_SANDBOX "_sandbox"

#define ADD_GLOBAL_TO_SANDBOX(x) \
  lua_pushliteral(L, x); \
  lua_getglobal(L, x); \
  lua_settable(L, -3);

namespace Lua {

void Sandbox::close(lua_State* L)
{
  delete *static_cast<StateData**>(lua_getextraspace(L)); // free state data
  lua_close(L);
}

SandboxPtr Sandbox::create(Script& script)
{
  lua_State* L = luaL_newstate();

  // load Lua baselib:
  lua_pushcfunction(L, luaopen_base);
  lua_pushliteral(L, "");
  lua_call(L, 1, 0);

  // create state data:
  *static_cast<StateData**>(lua_getextraspace(L)) = new StateData(script);

  // register types:
  Enum<DecoderProtocol>::registerType(L);
  Enum<Direction>::registerType(L);
  Enum<WorldEvent>::registerType(L);
  Set<WorldState>::registerType(L);
  Object::registerType(L);
  Method::registerType(L);

  // setup sandbox:
  lua_newtable(L);

  // add some Lua baselib functions to the sandbox:
  ADD_GLOBAL_TO_SANDBOX("assert")
  ADD_GLOBAL_TO_SANDBOX("type")
  ADD_GLOBAL_TO_SANDBOX("pairs")
  ADD_GLOBAL_TO_SANDBOX("ipairs")

  // set VERSION:
  lua_pushstring(L, STR(VERSION));
  lua_setfield(L, -2, "VERSION");

  // set CODENAME
  lua_pushstring(L, TRAINTASTIC_CODENAME);
  lua_setfield(L, -2, "CODENAME");

  // add world:
  push(L, std::static_pointer_cast<::Object>(script.world().lock()));
  lua_setfield(L, -2, "world");

  // add console:
  Console::push(L);
  lua_setfield(L, -2, "console");

  // add enum values:
  lua_newtable(L);
  Enum<DecoderProtocol>::registerValues(L);
  Enum<Direction>::registerValues(L);
  Enum<WorldEvent>::registerValues(L);
  ReadOnlyTable::setMetatable(L, -1);
  lua_setfield(L, -2, "enum");

  // add set values:
  lua_newtable(L);
  Set<WorldState>::registerValues(L);
  ReadOnlyTable::setMetatable(L, -1);
  lua_setfield(L, -2, "set");

  // let global _G point to itself:
  lua_pushliteral(L, "_G");
  lua_pushvalue(L, -2);
  lua_settable(L, -3);

  lua_setglobal(L, LUA_SANDBOX);

  return SandboxPtr(L, close);
}

Sandbox::StateData& Sandbox::getStateData(lua_State* L)
{
  return **static_cast<StateData**>(lua_getextraspace(L));
}

int Sandbox::getGlobal(lua_State* L, const char* name)
{
  lua_getglobal(L, LUA_SANDBOX); // get the sandbox
  lua_pushstring(L, name);
  const int type = lua_gettable(L, -2); // get item
  lua_insert(L, lua_gettop(L) - 1); // swap item and sandbox on the stack
  lua_pop(L, 1); // remove sandbox from the stack
  return type;
}

int Sandbox::pcall(lua_State* L, int nargs, int nresults, int errfunc)
{
  // check if the function has _ENV as first upvalue
  // if so, replace it by the sandbox
  // NOTE: functions which don't use any globals, don't have an _ENV !!
  assert(lua_isfunction(L, -(1 + nargs)));
  const char* name = lua_getupvalue(L, -(1 + nargs), 1);
  if(name)
    lua_pop(L, 1); // remove upvalue from stack
  if(name && strcmp(name, "_ENV") == 0)
  {
    lua_getglobal(L, LUA_SANDBOX); // get the sandbox
    assert(lua_istable(L, -1));
    if(!lua_setupvalue(L, -(2 + nargs), 1)) // change _ENV to the sandbox
    {
      assert(false); // should never happen
      lua_pop(L, 2 + nargs); // clear stack
      lua_pushliteral(L, "Internal error @ " __FILE__ ":" STR(__LINE__));
      return LUA_ERRRUN;
    }
  }
  return lua_pcall(L, nargs, nresults, errfunc);
}

}
