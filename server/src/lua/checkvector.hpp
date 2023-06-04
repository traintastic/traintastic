/**
 * server/src/lua/checkvector.hpp
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

#ifndef TRAINTASTIC_SERVER_LUA_CHECKVECTOR_HPP
#define TRAINTASTIC_SERVER_LUA_CHECKVECTOR_HPP

#include <lua.hpp>
#include <type_traits>
#include <vector>
#include <cstddef>
#include "check.hpp"

namespace Lua {

template<typename T>
std::vector<T> checkVector(lua_State* L, int index)
{
  luaL_checktype(L, index, LUA_TTABLE);
  std::vector<T> v;
  lua_len(L, index);
  const lua_Integer len = lua_tointeger(L, -1);
  lua_pop(L, 1);
  if(len > 0)
  {
    v.resize(len);
    for(lua_Integer i = 1; i <= len; i++)
    {
      lua_geti(L, index, i);
      v[i - 1] = check<T>(L, -1);
      lua_pop(L, 1);
    }
  }
  return v;
}

}

#endif
