/**
 * server/src/lua/log.cpp - Lua log wrapper
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021,2023 Reinder Feenstra
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

#include "log.hpp"
#include "readonlytable.hpp"
#include "sandbox.hpp"
#include "script.hpp"
#include "test.hpp"
#include "method.hpp"
#include "event.hpp"
#include "../log/log.hpp"

namespace Lua {

void Log::push(lua_State* L)
{
  lua_createtable(L, 0, 7);

  lua_pushcfunction(L, debug);
  lua_setfield(L, -2, "debug");
  lua_pushcfunction(L, info);
  lua_setfield(L, -2, "info");
  lua_pushcfunction(L, notice);
  lua_setfield(L, -2, "notice");
  lua_pushcfunction(L, warning);
  lua_setfield(L, -2, "warning");
  lua_pushcfunction(L, error);
  lua_setfield(L, -2, "error");
  lua_pushcfunction(L, critical);
  lua_setfield(L, -2, "critical");
  lua_pushcfunction(L, fatal);
  lua_setfield(L, -2, "fatal");

  ReadOnlyTable::wrap(L, -1);
}

int Log::log(lua_State* L, LogMessage code)
{
  const int top = lua_gettop(L);

  std::string message;
  for(int i = 1; i <= top; i++)
  {
    if(i != 1)
      message += " ";
    if(code == LogMessage::D9999_X)
    {
      switch(lua_type(L, i))
      {
        case LUA_TBOOLEAN:
          message += lua_toboolean(L, i) ? "true" : "false";
          break;

        case LUA_TNUMBER:
        case LUA_TSTRING:
          message += lua_tostring(L, i);
          break;

        case LUA_TUSERDATA:
          if(ObjectPtr object = test<::Object>(L, i))
            message += object->getClassId();
          else if(Method::test(L, i))
            message += "method";
          else if(Event::test(L, i))
            message += "event";
          else
          {
            lua_getglobal(L, "tostring");
            assert(lua_isfunction(L, -1));
            lua_pushvalue(L, i);
            if(lua_pcall(L, 1, 1, 0) == LUA_OK && lua_isstring(L, -1))
              message += lua_tostring(L, -1);
            else
              message += luaL_typename(L, i);
            lua_pop(L, 1);
          }
          break;

        default:
          message += luaL_typename(L, i);
          break;
      }
    }
    else
      message += luaL_checkstring(L, i);
  }

  ::Log::log(Sandbox::getStateData(L).script(), code, message);

  return 0;
}

}
