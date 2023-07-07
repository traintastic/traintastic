/**
 * server/src/lua/method.cpp - Lua method wrapper
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2022-2023 Reinder Feenstra
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
#include "check.hpp"
#include "error.hpp"
#include "to.hpp"
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
  auto& data = *static_cast<MethodData*>(luaL_checkudata(L, index, metaTableName));
  if(!data.object.expired())
    return data.method;

  errorDeadObject(L);
}

AbstractMethod* Method::test(lua_State* L, int index)
{
  auto* data = static_cast<MethodData*>(luaL_testudata(L, index, metaTableName));
  if(!data)
    return nullptr;
  if(!data->object.expired())
    return &data->method;

  errorDeadObject(L);
}

void Method::push(lua_State* L, AbstractMethod& value)
{
  new(lua_newuserdata(L, sizeof(MethodData))) MethodData(value);
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
  static_cast<MethodData*>(lua_touserdata(L, 1))->~MethodData();
  return 0;
}

int Method::__call(lua_State* L)
{
  AbstractMethod& method = check(L, 1);
  const int argc = static_cast<int>(method.argumentTypeInfo().size());

  if(lua_gettop(L) - 1 != argc)
    errorExpectedNArgumentsGotN(L, argc, lua_gettop(L) - 1);

  Arguments args;
  args.reserve(argc);

  if(argc > 0)
  {
    int index = 2;

    for(const auto& info : method.argumentTypeInfo())
    {
      switch(info.type)
      {
        case ValueType::Boolean:
          args.emplace_back(to<bool>(L, index));
          break;

        case ValueType::Enum:
          args.emplace_back(static_cast<int64_t>(checkEnum(L, index, info.enumName.data())));
          break;

        case ValueType::Integer:
          args.emplace_back(to<int64_t>(L, index));
          break;

        case ValueType::Float:
          args.emplace_back(to<double>(L, index));
          break;

        case ValueType::String:
          args.emplace_back(to<std::string>(L, index));
          break;

        case ValueType::Object:
          if(lua_isnil(L, index))
            args.emplace_back(ObjectPtr());
          else
            args.emplace_back(Lua::check<::Object>(L, index));
          break;

        case ValueType::Set:
          args.emplace_back(static_cast<int64_t>(checkSet(L, index, info.setName.data())));
          break;

        case ValueType::Invalid:
          assert(false);
          errorInternal(L);
      }
      index++;
    }
  }

  assert(args.size() == static_cast<Arguments::size_type>(argc));

  try
  {
    AbstractMethod::Result result = method.call(args);

    const auto& typeInfo = method.resultTypeInfo();
    switch(typeInfo.type)
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
