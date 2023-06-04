/**
 * server/src/lua/metatable.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_LUA_METATABLE_HPP
#define TRAINTASTIC_SERVER_LUA_METATABLE_HPP

#include <cassert>
#include <lua.hpp>
#include <string_view>
#include <cstring>

struct MetaTable
{
  static inline std::string_view getName(lua_State* L, int index)
  {
    if(lua_getmetatable(L, index))
    {
      lua_getfield(L, -1, "__name");
      size_t l;
      const char* s = lua_tolstring(L, -1, &l);
      lua_pop(L, 2);
      return {s, l};
    }
    return {};
  }

  static inline void clone(lua_State* L, const char* parent, const char* name)
  {
    if(luaL_getmetatable(L, parent) != 0) /*[[likely]]*/
    {
      luaL_newmetatable(L, name);
      // copy values of the parent metatable:
      lua_pushnil(L);
      while(lua_next(L, -3) != 0)
      {
        [[maybe_unused]] const char* s = lua_tostring(L, -2);
        if(std::strcmp(lua_tostring(L, -2), "__name") != 0)
        {
          lua_pushvalue(L, -2); // duplicate key, keep one for lua_next
          lua_insert(L, -2); // swap value and key
          lua_settable(L, -4);
        }
        else // don't copy __name
        {
          lua_pop(L, 1); // drop value, keep key for lua_next
        }
      }
      lua_insert(L, -2); // swap parent and new metatable
      lua_pop(L, 1); // pop parent
    }
    else
    {
      assert(false); // parent doesn't exists, must be registered first
      abort();
    }
  }
};

#endif
