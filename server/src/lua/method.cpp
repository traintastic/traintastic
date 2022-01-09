/**
 * server/src/lua/method.cpp - Lua method wrapper
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2022 Reinder Feenstra
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

#include "method.hpp"
#include "push.hpp"
#include "error.hpp"
#include "../core/abstractmethod.hpp"
#include "../core/object.hpp"

namespace Lua {

struct MethodData
{
  ObjectPtrWeak object;
  AbstractMethod& method;

  MethodData(AbstractMethod& _method) :
    object{_method.object().weak_from_this()},
    method{_method}
  {
  }
};

AbstractMethod& Method::check(lua_State* L, int index)
{
  MethodData& data = **static_cast<MethodData**>(luaL_checkudata(L, index, metaTableName));
  if(!data.object.expired())
    return data.method;

  errorDeadObject(L);
}

AbstractMethod* Method::test(lua_State* L, int index)
{
  MethodData** data = static_cast<MethodData**>(luaL_testudata(L, index, metaTableName));
  if(!data)
    return nullptr;
  if(!(**data).object.expired())
    return &(**data).method;

  errorDeadObject(L);
}

void Method::push(lua_State* L, AbstractMethod& value)
{
  *static_cast<MethodData**>(lua_newuserdata(L, sizeof(MethodData*))) = new MethodData(value);
  luaL_getmetatable(L, metaTableName);
  lua_setmetatable(L, -2);
}

void Method::registerType(lua_State* L)
{
  luaL_newmetatable(L, metaTableName);
  lua_pushcfunction(L, __gc);
  lua_setfield(L, -2, "__gc");
  lua_pushcfunction(L, __call);
  lua_setfield(L, -2, "__call");
  lua_pop(L, 1);
}

int Method::__gc(lua_State* L)
{
  delete *static_cast<MethodData**>(lua_touserdata(L, 1));
  return 0;
}

int Method::__call(lua_State* L)
{
  AbstractMethod& method = check(L, 1);
  const int argc = static_cast<int>(method.argumentCount());

  if(lua_gettop(L) - 1 != argc)
    errorExpectedNArgumentsGotN(L, argc, lua_gettop(L) - 1);

  Arguments args;
  args.reserve(argc);


  assert(argc == 0);

  try
  {
    AbstractMethod::Result result = method.call(args);

    switch(method.resultType())
    {
      case ValueType::Invalid:
        return 0; // no return value

      case ValueType::Boolean:
        Lua::push(L, std::get<bool>(result));
        return 1;

      case ValueType::Enum:
      case ValueType::Set:
        errorInternal(L); // not yet implemented

      case ValueType::Integer:
        Lua::push(L, std::get<int64_t>(result));
        return 1;

      case ValueType::Float:
        Lua::push(L, std::get<double>(result));
        return 1;

      case ValueType::String:
        Lua::push(L, std::get<std::string>(result));
        return 1;

      case ValueType::Object:
        Lua::push(L, std::get<ObjectPtr>(result));
        return 1;
    }

    assert(false);
    return 0;
  }
  catch(const std::exception& e)
  {
    errorException(L, e);
  }
}

}
