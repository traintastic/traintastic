/**
 * server/src/lua/classid.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_LUA_CLASS_HPP
#define TRAINTASTIC_SERVER_LUA_CLASS_HPP

#include <string_view>
#include <lua.hpp>
#include "../core/objectptr.hpp"

namespace Lua {

struct Class
{
  static void registerValues(lua_State* L);

  static void push(lua_State* L, std::string_view classId);
  static void push(lua_State* L, const ObjectPtr& object);

  template<class T>
  static void push(lua_State* L)
  {
    static_assert(std::is_base_of_v<::Object, T>);
    push(L, T::classId);
  }

  static int __tostring(lua_State* L);

  static int getClass(lua_State* L);
  static int create_throttle(lua_State* L);
};

}

#endif
