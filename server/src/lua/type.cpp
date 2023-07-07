/**
 * server/src/lua/type.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023 Reinder Feenstra
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

#include "type.hpp"
#include "test.hpp"
#include "class.hpp"
#include "sandbox.hpp"

namespace Lua {

int type(lua_State* L)
{
  const int tp = lua_type(L, 1);

  if(tp == LUA_TUSERDATA)
  {
    if(auto object = test<::Object>(L, 1))
    {
      lua_pushstring(L, "object");
      Class::push(L, object);
      return 2;
    }

    if(lua_getmetatable(L, 1))
    {
      lua_getfield(L, -1, "__name");
      const char* name = lua_tostring(L, -1);

      // check for enum:
      Sandbox::getGlobal(L, "enum");
      lua_getfield(L, -1, name);
      if(lua_istable(L, -1))
      {
        lua_pop(L, 2);
        lua_pushstring(L, "enum");
        lua_replace(L, -3);
        return 2;
      }
      lua_pop(L, 2);

      // check for set:
      Sandbox::getGlobal(L, "set");
      lua_getfield(L, -1, name);
      if(lua_istable(L, -1))
      {
        lua_pop(L, 2);
        lua_pushstring(L, "set");
        lua_replace(L, -3);
        return 2;
      }
      lua_pop(L, 2);
    }
  }

  lua_pushstring(L, lua_typename(L, tp));
  return 1;
}

}
