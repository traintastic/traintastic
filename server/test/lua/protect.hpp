/**
 * server/test/lua/protect.hpp
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

#ifndef TRAINTASTIC_SERVER_TEST_LUA_PROTECT_HPP
#define TRAINTASTIC_SERVER_TEST_LUA_PROTECT_HPP

#include <csetjmp>
#include <lua.hpp>

inline lua_State* newStateWithProtect()
{
  lua_State* L = luaL_newstate();
  *static_cast<void**>(lua_getextraspace(L)) = malloc(sizeof(jmp_buf));
  return L;
}

inline void closeStateWithProtect(lua_State* L)
{
  free(*static_cast<void**>(lua_getextraspace(L)));
  lua_close(L);
}

inline int protectPanic(lua_State* L)
{
  longjmp(*static_cast<jmp_buf*>(*static_cast<void**>(lua_getextraspace(L))), 1); // will never return
}

template<auto Func, class... Args>
bool protect(lua_State* L, Args... args)
{
  auto* oldPanic = lua_atpanic(L, protectPanic);

  bool success = true;
  if(setjmp(*static_cast<jmp_buf*>(*static_cast<void**>(lua_getextraspace(L)))) == 0)
    Func(L, args...);
  else
    success = false;

  lua_atpanic(L, oldPanic);

  return success;
}

template<auto Func, class R, class... Args>
bool protect(R& result, lua_State* L, Args... args)
{
  auto* oldPanic = lua_atpanic(L, protectPanic);

  bool success = true;
  if(setjmp(*static_cast<jmp_buf*>(*static_cast<void**>(lua_getextraspace(L)))) == 0)
    result = Func(L, args...);
  else
    success = false;

  lua_atpanic(L, oldPanic);

  return success;
}

#endif
