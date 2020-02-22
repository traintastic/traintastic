/**
 * server/src/lua/console.hpp - Lua console wrapper
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

#ifndef SERVER_LUA_CONSOLE_HPP
#define SERVER_LUA_CONSOLE_HPP

#include <lua.hpp>
#include "../core/console.hpp"

namespace Lua {

class Console
{
  private:
    static int log(lua_State* L, ::Console::Level level);

    static int debug(lua_State* L)  { return log(L, ::Console::Level::Debug); }
    static int info(lua_State* L) { return log(L, ::Console::Level::Info); }
    static int notice(lua_State* L)  { return log(L, ::Console::Level::Notice); }
    static int warning(lua_State* L)  { return log(L, ::Console::Level::Warning); }
    static int error(lua_State* L)  { return log(L, ::Console::Level::Error); }
    static int critical(lua_State* L)  { return log(L, ::Console::Level::Critical); }
    static int fatal(lua_State* L)  { return log(L, ::Console::Level::Fatal); }

  public:
    static void push(lua_State* L);

    static void log(lua_State* L, ::Console::Level level, const std::string& message);

    inline static void debug(lua_State* L, const std::string& message) { return log(L, ::Console::Level::Debug, message); }
    inline static void info(lua_State* L, const std::string& message) { return log(L, ::Console::Level::Info, message); }
    inline static void notice(lua_State* L, const std::string& message) { return log(L, ::Console::Level::Info, message); }
    inline static void warning(lua_State* L, const std::string& message) { return log(L, ::Console::Level::Warning, message); }
    inline static void error(lua_State* L, const std::string& message) { return log(L, ::Console::Level::Error, message); }
    inline static void critical(lua_State* L, const std::string& message) { return log(L, ::Console::Level::Critical, message); }
    inline static void fatal(lua_State* L, const std::string& message) { return log(L, ::Console::Level::Fatal, message); }
};

}

#endif
