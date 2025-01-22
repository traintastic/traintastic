/**
 * server/src/lua/object/interface.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_LUA_OBJECT_INTERFACE_HPP
#define TRAINTASTIC_SERVER_LUA_OBJECT_INTERFACE_HPP

#include <lua.hpp>
#include "../../hardware/interface/interface.hpp"

namespace Lua::Object {

class Interface
{
private:
  static int __index(lua_State* L);

  static int get_output(lua_State* L);

public:
  static constexpr char const* metaTableName = "object.interface";

  static void registerType(lua_State* L);

  static int index(lua_State* L, ::Interface& object);
};

}

#endif
