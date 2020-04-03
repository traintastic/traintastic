/**
 * server/src/lua/method.hpp - Lua method wrapper
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

#ifndef TRAINTASTIC_SERVER_LUA_METHOD_HPP
#define TRAINTASTIC_SERVER_LUA_METHOD_HPP

#include <lua.hpp>

class AbstractMethod;

namespace Lua {

class Method
{
  private:
    static int __gc(lua_State* L);
    static int __call(lua_State* L);

  public:
    static constexpr char const* metaTableName = "method";

    static AbstractMethod& check(lua_State* L, int index);
    static AbstractMethod* test(lua_State* L, int index);

    static void push(lua_State* L, AbstractMethod& value);

    static void registerType(lua_State* L);
};

}

#endif
