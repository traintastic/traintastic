/**
 * server/src/lua/error.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_LUA_ERROR_HPP
#define TRAINTASTIC_SERVER_LUA_ERROR_HPP

#include <lua.hpp>
#include <stdexcept>

namespace Lua {

// Note:
// All these functions call abort(), it's just a trick to let the compiler understand it is a noreturn function,
// Lua's error funtions aren't marked as noreturn functions, but they are.

[[noreturn]] inline void errorArgumentOutOfRange(lua_State* L, int arg) { luaL_argerror(L, arg, "out of range"); abort(); }
[[noreturn]] inline void errorArgumentExpectedObject(lua_State* L, int arg) { luaL_argerror(L, arg, "expected object"); abort(); }
[[noreturn]] inline void errorArgumentInvalidObject(lua_State* L, int arg) { luaL_argerror(L, arg, "invalid object"); abort(); }

[[noreturn]] inline void errorCantSetNonExistingProperty(lua_State* L) { luaL_error(L, "can't set non existing property"); abort(); }
[[noreturn]] inline void errorCantSetReadOnlyProperty(lua_State* L) { luaL_error(L, "can't set read only property"); abort(); }

[[noreturn]] inline void errorDeadObject(lua_State* L) { luaL_error(L, "dead object"); abort(); }

[[noreturn]] inline void errorExpectedNArgumentsGotN(lua_State* L, int expected, int got) { luaL_error(L, "expected %d arguments, got %d", expected, got); abort(); }
[[noreturn]] inline void errorExpectedNNArgumentsGotN(lua_State* L, int min, int max, int got) { luaL_error(L, "expected %d..%d arguments, got %d", min, max, got); abort(); }

[[noreturn]] inline void errorException(lua_State* L, const std::exception& e) { luaL_error(L, "exception: %s", e.what()); abort(); }

[[noreturn]] inline void errorGlobalNIsReadOnly(lua_State* L, const char* name) { luaL_error(L, "global %s is readonly", name); abort(); }

[[noreturn]] inline void errorInternal(lua_State* L) { luaL_error(L, "internal error"); abort(); }

[[noreturn]] inline void errorTableIsReadOnly(lua_State* L) { luaL_error(L, "table is readonly"); abort(); }

}

#endif
