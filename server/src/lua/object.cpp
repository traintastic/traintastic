/**
 * server/src/lua/object.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2024 Reinder Feenstra
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

#include "object.hpp"
#include "object/object.hpp"
#include "object/objectlist.hpp"
#include "object/interface.hpp"
#include "object/loconetinterface.hpp"

namespace Lua::Object {

constexpr char const* objectsGlobal = "objects";

void registerTypes(lua_State* L)
{
  Object::registerType(L);
  ObjectList::registerType(L);
  Interface::registerType(L);
  LocoNetInterface::registerType(L);

  // weak table for object userdata:
  lua_newtable(L);
  lua_newtable(L); // metatable
  lua_pushliteral(L, "__mode");
  lua_pushliteral(L, "v");
  lua_rawset(L, -3);
  lua_setmetatable(L, -2);
  lua_setglobal(L, objectsGlobal);
}

void push(lua_State* L, ::Object& value)
{
  push(L, value.shared_from_this());
}

void push(lua_State* L, const ObjectPtr& value)
{
  if(value)
  {
    lua_getglobal(L, objectsGlobal);
    lua_rawgetp(L, -1, value.get());
    if(lua_isnil(L, -1)) // object not in table
    {
      lua_pop(L, 1); // remove nil
      new(lua_newuserdata(L, sizeof(ObjectPtrWeak))) ObjectPtrWeak(value);

      if(dynamic_cast<::LocoNetInterface*>(value.get()))
        luaL_setmetatable(L, LocoNetInterface::metaTableName);
      else if(dynamic_cast<AbstractObjectList*>(value.get()))
        luaL_setmetatable(L, ObjectList::metaTableName);
      else
        luaL_setmetatable(L, Object::metaTableName);

      lua_pushvalue(L, -1); // copy userdata on stack
      lua_rawsetp(L, -3, value.get()); // add object to table
    }
    lua_insert(L, lua_gettop(L) - 1); // swap table and userdata
    lua_pop(L, 1); // remove table
  }
  else
    lua_pushnil(L);
}

}
