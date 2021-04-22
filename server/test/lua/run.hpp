/**
 * server/test/lua/run.hpp
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

#ifndef TRAINTASTIC_SERVER_TEST_LUA_RUN_HPP
#define TRAINTASTIC_SERVER_TEST_LUA_RUN_HPP

#include <lua.hpp>
#include <string_view>

inline bool run(lua_State* L, std::string_view code)
{
  return
    luaL_loadbuffer(L, code.data(), code.size(), "=") == LUA_OK &&
    lua_pcall(L, 0, LUA_MULTRET, 0) == LUA_OK;
}

#endif
