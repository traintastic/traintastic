/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2019-2026 Reinder Feenstra
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
#include "../check.hpp"
#include "../checkarguments.hpp"
#include "../push.hpp"
#include "../to.hpp"
#include "../method.hpp"
#include "../onchangedhandler.hpp"
#include "../sandbox.hpp"
#include "../event.hpp"
#include "../vectorproperty.hpp"
#include "../../core/object.hpp"
#include "../../core/abstractproperty.hpp"
#include "../../core/abstractvectorproperty.hpp"
#include "../../core/abstractmethod.hpp"
#include "../../core/abstractevent.hpp"
#include "../../utils/contains.hpp"
#include "../../utils/unreachable.hpp"

namespace Lua::Object {

void Object::registerType(lua_State* L)
{
  luaL_newmetatable(L, metaTableName);
  lua_pushcfunction(L, __gc);
  lua_setfield(L, -2, "__gc");
  lua_pushcfunction(L, __index);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, __newindex);
  lua_setfield(L, -2, "__newindex");
  lua_pop(L, 1);
}

void Object::pushPropertyValue(lua_State* L, AbstractProperty& property)
{
  if(!property.isScriptReadable())
  {
    lua_pushnil(L);
    return;
  }

  switch(property.type())
  {
    case ValueType::Boolean:
      Lua::push(L, property.toBool());
      return;

    case ValueType::Enum:
      // EnumName<T>::value assigned to the std::string_view is NUL terminated,
      // so it can be used as const char* however it is a bit tricky :)
      assert(*(property.enumName().data() + property.enumName().size()) == '\0');
      pushEnum(L, property.enumName().data(), static_cast<lua_Integer>(property.toInt64()));
      return;

    case ValueType::Integer:
      Lua::push(L, property.toInt64());
      return;

    case ValueType::Float:
      Lua::push(L, property.toDouble());
      return;

    case ValueType::String:
      Lua::push(L, property.toString());
      return;

    case ValueType::Object:
      push(L, property.toObject());
      return;

    case ValueType::Set:
      // set_name<T>::value assigned to the std::string_view is NUL terminated,
      // so it can be used as const char* however it is a bit tricky :)
      assert(*(property.setName().data() + property.setName().size()) == '\0');
      pushSet(L, property.setName().data(), static_cast<lua_Integer>(property.toInt64()));
      return;

    case ValueType::Invalid:
      break;
  }

  assert(false);
  lua_pushnil(L);
}

int Object::index(lua_State* L, ::Object& object)
{
  const auto key = to<std::string_view>(L, 2);

  LUA_OBJECT_METHOD(on_changed)

  if(InterfaceItem* item = object.getItem(key))
  {
    if(auto* property = dynamic_cast<AbstractProperty*>(item))
    {
      pushPropertyValue(L, *property);
    }
    else if(auto* vectorProperty = dynamic_cast<AbstractVectorProperty*>(item))
    {
      if(vectorProperty->isScriptReadable())
        VectorProperty::push(L, *vectorProperty);
      else
        lua_pushnil(L);
    }
    else if(auto* method = dynamic_cast<AbstractMethod*>(item))
    {
      if(method->isScriptCallable())
        Method::push(L, *method);
      else
        lua_pushnil(L);
    }
    else if(auto* event = dynamic_cast<AbstractEvent*>(item))
    {
      if(event->isScriptable())
        Event::push(L, *event);
      else
        lua_pushnil(L);
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

int Object::newindex(lua_State* L, ::Object& object)
{
  const auto key = to<std::string_view>(L, 2);

  if(AbstractProperty* property = object.getProperty(key))
  {
    if(!property->isScriptWriteable() || !property->isWriteable())
      errorCantSetReadOnlyProperty(L);

    try
    {
      switch(property->type())
      {
        case ValueType::Boolean:
          property->fromBool(Lua::check<bool>(L, 3));
          break;

        case ValueType::Enum:
          // EnumName<T>::value assigned to the std::string_view is NUL terminated,
          // so it can be used as const char* however it is a bit tricky :)
          assert(*(property->enumName().data() + property->enumName().size()) == '\0');
          property->fromInt64(checkEnum(L, 3, property->enumName().data()));
          break;

        case ValueType::Integer:
          property->fromInt64(Lua::check<int64_t>(L, 3));
          break;

        case ValueType::Float:
          property->fromDouble(Lua::check<double>(L, 3));
          break;

        case ValueType::String:
          property->fromString(Lua::check<std::string>(L, 3));
          break;

        case ValueType::Object:
          property->fromObject(check<::Object>(L, 3));
          break;

        default:
          assert(false);
          errorInternal(L);
      }
      return 0;
    }
    catch(const std::exception& e)
    {
      errorException(L, e);
    }
  }

  errorCantSetNonExistingProperty(L);
}

int Object::__gc(lua_State* L)
{
  static_cast<ObjectPtrWeak*>(lua_touserdata(L, 1))->~ObjectPtrWeak();
  return 0;
}

int Object::__index(lua_State* L)
{
  return index(L, *check<::Object>(L, 1));
}

int Object::__newindex(lua_State* L)
{
  return newindex(L, *check<::Object>(L, 1));
}

int Object::on_changed(lua_State* L)
{
  checkArguments(L, 1, 3);
  if(lua_isfunction(L, 1))
  {
    checkArguments(L, 1, 2);
  }
  else
  {
    checkArguments(L, 2, 3);
    luaL_checktype(L, 2, LUA_TFUNCTION);
  }

  {
    auto object = check<::Object>(L, lua_upvalueindex(1));
    int functionIndex = 1;

    std::vector<std::string> properties;

    if(!lua_isfunction(L, 1))
    {
      functionIndex = 2;
      if(lua_type(L, 1) == LUA_TSTRING)
      {
        properties.emplace_back(to<std::string>(L, 1));
      }
      else if(lua_istable(L, 1))
      {
        lua_len(L, 1);
        const lua_Integer len = lua_tointeger(L, -1);
        lua_pop(L, 1);

        if(len == 0)
        {
          lua_pushliteral(L, "property list is empty");
          goto error;
        }

        for(lua_Integer i = 1; i <= len; ++i)
        {
          lua_geti(L, 1, i);
          auto name = to<std::string>(L, -1);

          if(!contains(properties, name))
          {
            properties.emplace_back(std::move(name));
          }
          else
          {
            // TODO: warning?
          }
          lua_pop(L, 1);
        }
      }
      else
      {
        lua_pushliteral(L, "single property name or non empty table with names required");
        goto error;
      }
      assert(!properties.empty());

      // check if properties exist and are exposed to Lua:
      for(const auto& name : properties)
      {
        auto* property = dynamic_cast<BaseProperty*>(object->getItem(name));
        if(!property || !property->isScriptReadable())
        {
          lua_pushfstring(L, "unknown property: %s", name.c_str());
          goto error;
        }
      }
    }

    auto handler = std::make_shared<OnChangedHandler>(*object, L, functionIndex);
    if(!properties.empty())
    {
      handler->setFilter(properties);
    }
    OnChangedHandler::push(L, *handler);
    Sandbox::getStateData(L).registerOnChangedHandler(std::move(handler));

    return 1;
  }

error:
  lua_error(L); // !!! lua_error() uses longjmp; C++ stack unwinding does not occur and destructors are NOT called !!!
  unreachable();
}

}
