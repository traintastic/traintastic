/**
 * server/src/lua/vectorproperty.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#include "vectorproperty.hpp"
#include "push.hpp"
#include "error.hpp"
#include "to.hpp"
#include "../core/abstractvectorproperty.hpp"
#include "../core/object.hpp"

namespace Lua {

struct VectorPropertyData
{
  ObjectPtrWeak object;
  AbstractVectorProperty& vectorproperty;

  VectorPropertyData(AbstractVectorProperty& _vectorproperty) :
    object{_vectorproperty.object().weak_from_this()},
    vectorproperty{_vectorproperty}
  {
  }
};

AbstractVectorProperty& VectorProperty::check(lua_State* L, int index)
{
  auto& data = *static_cast<VectorPropertyData*>(luaL_checkudata(L, index, metaTableName));
  if(!data.object.expired())
    return data.vectorproperty;

  errorDeadObject(L);
}

AbstractVectorProperty* VectorProperty::test(lua_State* L, int index)
{
  auto* data = static_cast<VectorPropertyData*>(luaL_testudata(L, index, metaTableName));
  if(!data)
    return nullptr;
  if(!data->object.expired())
    return &data->vectorproperty;

  errorDeadObject(L);
}

void VectorProperty::push(lua_State* L, AbstractVectorProperty& value)
{
  new(lua_newuserdata(L, sizeof(VectorPropertyData))) VectorPropertyData(value);
  luaL_getmetatable(L, metaTableName);
  lua_setmetatable(L, -2);
}

void VectorProperty::registerType(lua_State* L)
{
  luaL_newmetatable(L, metaTableName);
  lua_pushcfunction(L, __index);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, __len);
  lua_setfield(L, -2, "__len");
  lua_pushcfunction(L, __gc);
  lua_setfield(L, -2, "__gc");
  lua_pop(L, 1);
}

int VectorProperty::__index(lua_State* L)
{
  auto& vectorProperty = check(L, 1);
  assert(vectorProperty.isScriptReadable());

  int ok;
  const lua_Integer index = lua_tointegerx(L, 2, &ok) - 1;

  if(ok && index >= 0 && static_cast<lua_Unsigned>(index) < vectorProperty.size())
  {
    switch(vectorProperty.type())
    {
      case ValueType::Boolean:
        Lua::push(L, vectorProperty.getBool(index));
        break;

      case ValueType::Enum:
        // EnumName<T>::value assigned to the std::string_view is NUL terminated,
        // so it can be used as const char* however it is a bit tricky :/
        assert(*(vectorProperty.enumName().data() + vectorProperty.enumName().size()) == '\0');
        pushEnum(L, vectorProperty.enumName().data(), static_cast<lua_Integer>(vectorProperty.getInt64(index)));
        break;

      case ValueType::Integer:
        Lua::push(L, vectorProperty.getInt64(index));
        break;

      case ValueType::Float:
        Lua::push(L, vectorProperty.getDouble(index));
        break;

      case ValueType::String:
        Lua::push(L, vectorProperty.getString(index));
        break;

      case ValueType::Object:
        Lua::push(L, vectorProperty.getObject(index));
        break;

      case ValueType::Set:
        // set_name<T>::value assigned to the std::string_view is NUL terminated,
        // so it can be used as const char* however it is a bit tricky :/
        assert(*(vectorProperty.setName().data() + vectorProperty.setName().size()) == '\0');
        pushSet(L, vectorProperty.setName().data(), static_cast<lua_Integer>(vectorProperty.getInt64(index)));
        break;

      default:
        assert(false);
        lua_pushnil(L);
        break;
    }
  }
  else
    lua_pushnil(L);

  return 1;
}

int VectorProperty::__len(lua_State* L)
{
  auto& vectorProperty = check(L, 1);
  assert(vectorProperty.isScriptReadable());
  lua_pushinteger(L, static_cast<lua_Integer>(vectorProperty.size()));
  return 1;
}

int VectorProperty::__gc(lua_State* L)
{
  static_cast<VectorPropertyData*>(lua_touserdata(L, 1))->~VectorPropertyData();
  return 0;
}

}
