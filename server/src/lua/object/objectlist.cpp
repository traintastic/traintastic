/**
 * server/src/lua/object/objectlist.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2023 Reinder Feenstra
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

#include "objectlist.hpp"
#include "object.hpp"
#include "../check.hpp"
#include "../to.hpp"
#include "../push.hpp"
#include "../metatable.hpp"

namespace Lua::Object {

void ObjectList::registerType(lua_State* L)
{
  MetaTable::clone(L, Object::metaTableName, metaTableName);
  lua_pushcfunction(L, __index);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, __len);
  lua_setfield(L, -2, "__len");
  lua_pop(L, 1);
}

int ObjectList::index(lua_State* L, ::AbstractObjectList& object)
{
  // handle list[index]:
  lua_Integer index;
  if(to(L, 2, index))
  {
    if(index >= 1 && index <= object.length)
      push(L, object.getObject(static_cast<uint32_t>(index - 1)));
    else
      lua_pushnil(L);
    return 1;
  }

  return Object::index(L, object);
}

int ObjectList::__len(lua_State* L)
{
  Lua::push(L, check<::AbstractObjectList>(L, 1)->length.value());
  return 1;
}

int ObjectList::__index(lua_State* L)
{
  return index(L, *check<::AbstractObjectList>(L, 1));
}

}
