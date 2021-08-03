/**
 * server/src/lua/log.hpp - Lua log wrapper
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_LUA_LOG_HPP
#define TRAINTASTIC_SERVER_LUA_LOG_HPP

#include <lua.hpp>
#include "../log/log.hpp"

namespace Lua {

class Log
{
  private:
    static int log(lua_State* L, LogMessage code);

    static int debug(lua_State* L)  { return log(L, LogMessage::D9999_X); }
    static int info(lua_State* L) { return log(L, LogMessage::I9999_X); }
    static int notice(lua_State* L) { return log(L, LogMessage::N9999_X); }
    static int warning(lua_State* L) { return log(L, LogMessage::W9999_X); }
    static int error(lua_State* L) { return log(L, LogMessage::E9999_X); }
    static int critical(lua_State* L) { return log(L, LogMessage::C9999_X); }
    static int fatal(lua_State* L) { return log(L, LogMessage::F9999_X); }

  public:
    static void push(lua_State* L);
};

}

#endif
