/**
 * server/src/lua/object.cpp - Lua object wrapper
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

#include "object.hpp"
#include "push.hpp"
#include "check.hpp"
#include "method.hpp"
#include "../core/object.hpp"
#include "../core/abstractproperty.hpp"
#include "../core/abstractmethod.hpp"

namespace Lua {

ObjectPtr Object::check(lua_State* L, int index)
{
  ObjectPtrWeak& data = **static_cast<ObjectPtrWeak**>(luaL_checkudata(L, index, metaTableName));
  if(ObjectPtr object = data.lock())
    return object;
  else
  {
    luaL_error(L, "dead object");
    abort(); // never happens, luaL_error doesn't return
  }
}

ObjectPtr Object::test(lua_State* L, int index)
{
  ObjectPtrWeak** data = static_cast<ObjectPtrWeak**>(luaL_testudata(L, index, metaTableName));
  if(!data)
    return ObjectPtr();
  else if(ObjectPtr object = (**data).lock())
    return object;
  else
  {
    luaL_error(L, "dead object");
    abort(); // never happens, luaL_error doesn't return
  }
}

void Object::push(lua_State* L, const ObjectPtr& value)
{
  if(value)
  {
    lua_rawgetp(L, LUA_REGISTRYINDEX, metaTableName);
    lua_rawgetp(L, -1, value.get());
    if(lua_isnil(L, -1))
    {
      *static_cast<ObjectPtrWeak**>(lua_newuserdata(L, sizeof(ObjectPtrWeak*))) = new ObjectPtrWeak(value);
      luaL_setmetatable(L, metaTableName);
      lua_pushvalue(L, -1); // copy ud
      lua_rawsetp(L, -3, value.get()); // add object to table
    }
    lua_insert(L, lua_gettop(L) - 1); // swap table and userdata
    lua_pop(L, 1); // remove table
  }
  else
    lua_pushnil(L);
}

void Object::registerType(lua_State* L)
{
  // meata rable for object userdata:
  luaL_newmetatable(L, metaTableName);
  lua_pushcfunction(L, __gc);
  lua_setfield(L, -2, "__gc");
  lua_pushcfunction(L, __index);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, __newindex);
  lua_setfield(L, -2, "__newindex");
  lua_pop(L, 1);

  // weak table for object userdata:
  lua_newtable(L);
  lua_newtable(L); // metatable
  lua_pushliteral(L, "__mode");
  lua_pushliteral(L, "v");
  lua_rawset(L, -3);
  lua_setmetatable(L, -2);
  lua_rawsetp(L, LUA_REGISTRYINDEX, metaTableName);
}

int Object::__gc(lua_State* L)
{
  delete *static_cast<ObjectPtrWeak**>(lua_touserdata(L, 1));
  return 0;
}

int Object::__index(lua_State* L)
{
  ObjectPtr object{check(L, 1)};
  const std::string name{lua_tostring(L, 2)};

  if(InterfaceItem* item = object->getItem(name))
  {
    // TODO: test scriptable

    if(AbstractProperty* property = dynamic_cast<AbstractProperty*>(item))
    {
      switch(property->type())
      {
        case ValueType::Boolean:
          Lua::push(L, property->toBool());
          break;

        //case ValueType::Enum:

        case ValueType::Integer:
          Lua::push(L, property->toInt64());
          break;

        case ValueType::Float:
          Lua::push(L, property->toDouble());
          break;

        case ValueType::String:
          Lua::push(L, property->toString());
          break;

        case ValueType::Object:
          push(L, property->toObject());
          break;

        default:
          assert(false);
          lua_pushnil(L);
          break;
      }
    }
    else if(AbstractMethod* method = dynamic_cast<AbstractMethod*>(item))
    {
      Method::push(L, *method);
    }
    else
    {
      assert(false); // it must be a property or method
      lua_pushnil(L);
    }
  }
  else
    lua_pushnil(L);

  return 1;
}

int Object::__newindex(lua_State* L)
{
  ObjectPtr object{check(L, 1)};
  const std::string name{lua_tostring(L, 2)};

  if(AbstractProperty* property = object->getProperty(name))
  {
    // TODO: test scriptable

    if(!property->isWriteable())
      return luaL_error(L, "can't set read only property");

    try
    {
      switch(property->type())
      {
        case ValueType::Boolean:
          property->fromBool(Lua::check<bool>(L, 2));
          break;

        case ValueType::Integer:
          property->fromInt64(Lua::check<int64_t>(L, 2));
          break;

        case ValueType::Float:
          property->fromDouble(Lua::check<double>(L, 2));
          break;

        case ValueType::String:
          property->fromString(Lua::check<std::string>(L, 2));
          break;

        case ValueType::Object:
          property->fromObject(check(L, 2));
          break;

        default:
          assert(false);
          return luaL_error(L, "internal error");
      }
      return 0;
    }
    catch(const std::exception& e)
    {
      return luaL_error(L, "%s", e.what());
    }
  }

  return luaL_error(L, "can't set non existing property");
}

}
