/**
 * server/src/lua/readonlytable.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_LUA_READONLYTABLE_HPP
#define TRAINTASTIC_SERVER_LUA_READONLYTABLE_HPP

#include <cassert>
#include <lua.hpp>
#include "error.hpp"

namespace Lua {

class ReadOnlyTable
{
  private:
    static int __newindex(lua_State* L)
    {
      errorTableIsReadOnly(L);
    }

    static int __pairs(lua_State* L)
    {
      lua_getglobal(L, "next");
      assert(lua_isfunction(L, -1));
      lua_getmetatable(L, 1);
      assert(lua_istable(L, -1));
      lua_getfield(L, -1, "__index");
      assert(lua_istable(L, -1));
      lua_replace(L, lua_gettop(L) - 1); // replace metatable by source table
      return 2;
    }

  public:
    static void wrap(lua_State* L, int index)
    {
      assert(lua_istable(L, index));
      lua_newtable(L); // create wrapper table
      lua_newtable(L); // metatable for wrapper table
      if(index < 0)
        index -= 2; // correct index if relative
      lua_pushvalue(L, index); // copy source table (ref)
      lua_setfield(L, -2, "__index");
      lua_pushcfunction(L, __newindex);
      lua_setfield(L, -2, "__newindex");
      lua_pushcfunction(L, __pairs);
      lua_setfield(L, -2, "__pairs");
      lua_pushboolean(L, 0);
      lua_setfield(L, -2, "__metatable");
      lua_setmetatable(L, -2); // set metatable @ wrapper table
      if(index < 0)
        index++; // correct index if relative
      lua_replace(L, index); // replace source table by wrapper table
    }
};

}

#endif
