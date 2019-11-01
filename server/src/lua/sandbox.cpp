/**
 * server/src/lua/sandbox.cpp - Lua sandbox
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019 Reinder Feenstra
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
#include "../enum/decoderprotocol.hpp"
#include "../enum/direction.hpp"
// TODO: #include "object.hpp"

#define ADD_GLOBAL_TO_SANDBOX(x) \
  lua_pushliteral(L, x); \
  lua_getglobal(L, x); \
  lua_settable(L, -3);

namespace Lua {

lua_State* newSandbox()
{
  lua_State* L = luaL_newstate();

  // load Lua baselib:
  lua_pushcfunction(L, luaopen_base);
  lua_pushliteral(L, "");
  lua_call(L, 1, 0);

  // register types:
  Enum<DecoderProtocol>::registerType(L);
  Enum<Direction>::registerType(L);
  // TODO: Object::registerType(L);

  // setup sandbox:
  lua_newtable(L);

  // add some Lua baselib functions to the sandbox:
  ADD_GLOBAL_TO_SANDBOX("assert")
  ADD_GLOBAL_TO_SANDBOX("type")
  ADD_GLOBAL_TO_SANDBOX("pairs")
  ADD_GLOBAL_TO_SANDBOX("ipairs")

  // add enum values:
  lua_newtable(L);
  Enum<DecoderProtocol>::registerValues(L);
  Enum<Direction>::registerValues(L);
  ReadOnlyTable::setMetatable(L, -1);
  lua_setfield(L, -2, "enum");

  // let global _G point to itself:
  lua_pushliteral(L, "_G");
  lua_pushvalue(L, -2);
  lua_settable(L, -3);

  lua_setglobal(L, LUA_SANDBOX);

  return L;
}

void closeSandbox(lua_State* L)
{
  lua_close(L);
}

}
