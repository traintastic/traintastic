/**
 * server/src/lua/sandbox.cpp - Lua sandbox
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2023 Reinder Feenstra
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
#include "method.hpp"
#include "event.hpp"
#include "eventhandler.hpp"
#include "log.hpp"
#include "class.hpp"
#include "to.hpp"
#include "type.hpp"
#include "object.hpp"
#include "enums.hpp"
#include "sets.hpp"
#include "getversion.hpp"
#include "script.hpp"
#include "vectorproperty.hpp"
#include <version.hpp>
#include <traintastic/utils/str.hpp>
#include "../world/world.hpp"

#define LUA_SANDBOX "_sandbox"
#define LUA_SANDBOX_GLOBALS "_sandbox_globals"

constexpr std::array<std::string_view, 23> readOnlyGlobals = {{
  // Lua baselib:
  "assert",
  "type",
  "pairs",
  "ipairs",
  "next",
  "tonumber",
  "tostring",
  "_G",
  // Lua libs:
  LUA_MATHLIBNAME,
  LUA_STRLIBNAME,
  LUA_TABLIBNAME,
  // Constants:
  "VERSION",
  "VERSION_MAJOR",
  "VERSION_MINOR",
  "VERSION_PATCH",
  "CODENAME",
  "LUA_VERSION",
  // Objects:
  "world",
  "log",
  // Functions:
  "is_instance",
  // Type info:
  "class",
  "enum",
  "set",
}};

static void addExtensions(lua_State* L, std::initializer_list<std::pair<const char*, lua_CFunction>> extensions)
{
  assert(lua_istable(L, -1));
  for(auto [name, func] : extensions)
  {
    lua_pushcfunction(L, func);
    lua_setfield(L, -2, name);
  }
}

static void addBaseLib(lua_State* L, std::initializer_list<const char*> names, std::initializer_list<std::pair<const char*, lua_CFunction>> extensions = {})
{
  // load Lua baselib:
  lua_pushcfunction(L, luaopen_base);
  lua_pushliteral(L, "");
  lua_call(L, 1, 0);

  for(const char* name : names)
  {
    lua_getglobal(L, name);
    assert(!lua_isnil(L, -1));
    lua_setfield(L, -2, name);
  }

  addExtensions(L, extensions);
}

static void addLib(lua_State* L, const char* libraryName, lua_CFunction openFunction, std::initializer_list<const char*> names, std::initializer_list<std::pair<const char*, lua_CFunction>> extensions = {})
{
  lua_createtable(L, 0, names.size() + extensions.size());

  luaL_requiref(L, libraryName, openFunction, 1);

  for(const char* name : names)
  {
    lua_getfield(L, -1, name);
    assert(!lua_isnil(L, -1));
    lua_setfield(L, -3, name);
  }

  lua_pop(L, 1); // pop lib

  addExtensions(L, extensions);

  Lua::ReadOnlyTable::wrap(L, -1);
  lua_setfield(L, -2, libraryName);
}

namespace Lua {

void Sandbox::close(lua_State* L)
{
  delete *static_cast<StateData**>(lua_getextraspace(L)); // free state data
  lua_close(L);
}

int Sandbox::__index(lua_State* L)
{
  lua_getglobal(L, LUA_SANDBOX_GLOBALS);
  lua_replace(L, 1);

  lua_rawget(L, 1);

  return 1;
}

int Sandbox::__newindex(lua_State* L)
{
  if(std::find(readOnlyGlobals.begin(), readOnlyGlobals.end(), to<std::string_view>(L, 2)) != readOnlyGlobals.end())
    errorGlobalNIsReadOnly(L, lua_tostring(L, 2));

  lua_getglobal(L, LUA_SANDBOX_GLOBALS);
  lua_replace(L, 1);

  lua_rawset(L, 1);

  return 0;
}

SandboxPtr Sandbox::create(Script& script)
{
  lua_State* L = luaL_newstate();

  // create state data:
  *static_cast<StateData**>(lua_getextraspace(L)) = new StateData(script);

  // register types:
  Enums::registerTypes<LUA_ENUMS>(L);
  Sets::registerTypes<LUA_SETS>(L);
  Object::registerTypes(L);
  VectorProperty::registerType(L);
  Method::registerType(L);
  Event::registerType(L);

  // setup sandbox:
  lua_newtable(L);
  luaL_newmetatable(L, LUA_SANDBOX);
  lua_pushcfunction(L, __index);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, __newindex);
  lua_setfield(L, -2, "__newindex");
  lua_setmetatable(L, -2);
  lua_setglobal(L, LUA_SANDBOX);

  // setup globals:
  lua_newtable(L);

  // add standard Lua lib functions and extensions/replacements:
  addBaseLib(L,
    {
      "assert", "pairs", "ipairs", "next", "tonumber", "tostring",
    },
    {
      {"type", type},
    });
  addLib(L, LUA_MATHLIBNAME, luaopen_math, {
    "abs", "acos", "asin", "atan", "ceil", "cos", "deg", "exp",
    "floor", "fmod", "huge", "log", "max", "maxinteger", "min", "mininteger",
    "modf", "pi", "rad", "random", "randomseed", "sin", "sqrt", "tan",
    "tointeger", "type", "ult",
    });
  addLib(L, LUA_STRLIBNAME, luaopen_string, {
    "byte", "char", "find", "format", "gmatch", "gsub", "len", "lower",
    "match", "pack", "packsize", "rep", "reverse", "sub", "unpack", "upper",
    });
  addLib(L, LUA_TABLIBNAME, luaopen_table, {
    "concat", "insert", "pack", "unpack", "remove", "move", "sort",
    });

  // set VERSION:
  lua_pushstring(L, TRAINTASTIC_VERSION_FULL);
  lua_setfield(L, -2, "VERSION");
  lua_pushinteger(L, TRAINTASTIC_VERSION_MAJOR);
  lua_setfield(L, -2, "VERSION_MAJOR");
  lua_pushinteger(L, TRAINTASTIC_VERSION_MINOR);
  lua_setfield(L, -2, "VERSION_MINOR");
  lua_pushinteger(L, TRAINTASTIC_VERSION_PATCH);
  lua_setfield(L, -2, "VERSION_PATCH");

  // set LUA_VERSION
  push(L, getVersion());
  lua_setfield(L, -2, "LUA_VERSION");

  // add world:
  push(L, script.world().shared_from_this());
  lua_setfield(L, -2, "world");

  // add logger:
  Log::push(L);
  lua_setfield(L, -2, "log");

  // add class types:
  lua_newtable(L);
  Class::registerValues(L);
  ReadOnlyTable::wrap(L, -1);
  lua_setfield(L, -2, "class");

  // add enum values:
  lua_newtable(L);
  Enums::registerValues<LUA_ENUMS>(L);
  ReadOnlyTable::wrap(L, -1);
  lua_setfield(L, -2, "enum");

  // add set values:
  lua_newtable(L);
  Sets::registerValues<LUA_SETS>(L);
  ReadOnlyTable::wrap(L, -1);
  lua_setfield(L, -2, "set");

  // let global _G point to the sandbox:
  lua_getglobal(L, LUA_SANDBOX);
  lua_setfield(L, -2, "_G");

  lua_setglobal(L, LUA_SANDBOX_GLOBALS);

  return SandboxPtr(L, close);
}

Sandbox::StateData& Sandbox::getStateData(lua_State* L)
{
  return **static_cast<StateData**>(lua_getextraspace(L));
}

int Sandbox::getGlobal(lua_State* L, const char* name)
{
  lua_getglobal(L, LUA_SANDBOX_GLOBALS); // get the sandbox
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

  // limit execution time:
  // Only start for first pcall, a pcall can cause another pcall.
  const bool firstCall = lua_gethook(L) == nullptr;
  if(firstCall)
  {
    auto& stateData = getStateData(L);
    stateData.pcallStart = std::chrono::steady_clock::now();
    stateData.pcallExecutionTimeViolation = false;
    lua_sethook(L, hook, LUA_MASKCOUNT, 1000);
  }

  const int r = lua_pcall(L, nargs, nresults, errfunc);

  if(firstCall)
  {
    if(!getStateData(L).pcallExecutionTimeViolation)
    {
      const auto duration = (std::chrono::steady_clock::now() - getStateData(L).pcallStart);
      if(duration >= pcallDurationWarning)
      {
        ::Log::log(getStateData(L).script(), LogMessage::W9001_EXECUTION_TOOK_X_US, std::chrono::duration_cast<std::chrono::microseconds>(duration).count());
      }
    }
    lua_sethook(L, nullptr, 0, 0);
  }

  return r;
}

void Sandbox::hook(lua_State* L, lua_Debug* /*ar*/)
{
  if((std::chrono::steady_clock::now() - getStateData(L).pcallStart) > pcallDurationMax)
  {
    getStateData(L).pcallExecutionTimeViolation = true;
    luaL_error(L, "Exceeded maximum execution time.");
  }
}

Sandbox::StateData::~StateData()
{
  while(!m_eventHandlers.empty())
  {
    auto handler = m_eventHandlers.begin()->second;
    m_eventHandlers.erase(m_eventHandlers.begin());
    handler->disconnect();
  }
}

}
