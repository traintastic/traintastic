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

inline static jmp_buf protectPanicJump;

inline int protectPanic(lua_State*)
{
  longjmp(protectPanicJump, 1); // will never return
}

template<auto Func, class... Args>
bool protect(lua_State* L, Args... args)
{
  auto* oldPanic = lua_atpanic(L, protectPanic);

  bool success = true;
  if(setjmp(protectPanicJump) == 0)
    Func(L, args...);
  else
    success = false;

  lua_atpanic(L, oldPanic);

  return success;
}

#endif
