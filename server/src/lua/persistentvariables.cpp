/**
 * server/src/lua/persistent.cpp
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

#include "persistentvariables.hpp"
#include <vector>
#include "enums.hpp"
#include "error.hpp"
#include "event.hpp"
#include "method.hpp"
#include "object.hpp"
#include "sandbox.hpp"
#include "script.hpp"
#include "sets.hpp"
#include "test.hpp"
#include "vectorproperty.hpp"
#include "../core/abstractmethod.hpp"
#include "../utils/contains.hpp"
#include "../world/world.hpp"

namespace Lua {

static void checkTableRecursion(lua_State* L, std::vector<int>& indices)
{
  const int index = indices.back();
  assert(lua_istable(L, index));

  lua_pushnil(L);
  while(lua_next(L, index))
  {
    if(lua_istable(L, -1))
    {
      if(std::find_if(indices.begin(), indices.end(),
        [L](int idx)
        {
          return lua_rawequal(L, idx, -1);
        }) != indices.end())
      {
        errorTableContainsRecursion(L);
      }

      indices.push_back(lua_gettop(L));
      checkTableRecursion(L, indices);
      indices.pop_back();
    }
    lua_pop(L, 1);
  }
}

static void checkTableRecursion(lua_State* L, int index)
{
  assert(lua_istable(L, index));

  std::vector<int> indices;
  indices.push_back(lua_absindex(L, index));

  checkTableRecursion(L, indices);
}

static const char* metaTableName = "pv";

struct PersistentVariablesData
{
  int registryIndex;
};

void PersistentVariables::registerType(lua_State* L)
{
  luaL_newmetatable(L, metaTableName);
  lua_pushcfunction(L, __index);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, __newindex);
  lua_setfield(L, -2, "__newindex");
  lua_pushcfunction(L, __pairs);
  lua_setfield(L, -2, "__pairs");
  lua_pushcfunction(L, __len);
  lua_setfield(L, -2, "__len");
  lua_pushcfunction(L, __gc);
  lua_setfield(L, -2, "__gc");
  lua_pop(L, 1);
}

bool PersistentVariables::test(lua_State* L, int index)
{
  return luaL_testudata(L, index, metaTableName);
}

void PersistentVariables::push(lua_State* L)
{
  auto* pv = static_cast<PersistentVariablesData*>(lua_newuserdatauv(L, sizeof(PersistentVariablesData), 1));
  luaL_getmetatable(L, metaTableName);
  lua_setmetatable(L, -2);
  lua_newtable(L);
  pv->registryIndex = luaL_ref(L, LUA_REGISTRYINDEX);
}

void PersistentVariables::push(lua_State* L, const nlohmann::json& value)
{
  switch(value.type())
  {
    case nlohmann::json::value_t::null:
      return lua_pushnil(L);

    case nlohmann::json::value_t::boolean:
      return lua_pushboolean(L, static_cast<bool>(value));

    case nlohmann::json::value_t::number_integer:
    case nlohmann::json::value_t::number_unsigned:
      return lua_pushinteger(L, value);

    case nlohmann::json::value_t::number_float:
      return lua_pushnumber(L, value);

    case nlohmann::json::value_t::string:
    {
      const std::string s = value;
      lua_pushlstring(L, s.data(), s.size());
      return;
    }
    case nlohmann::json::value_t::object:
    {
      if(value.contains("type"))
      {
        const std::string type = value["type"];
        if(type == "object")
        {
          if(value.contains("id"))
          {
            const std::string id = value["id"];
            if(auto object = Sandbox::getStateData(L).script().world().getObjectByPath(id))
            {
              return Object::push(L, object);
            }
            return lua_pushnil(L);
          }
        }
        else if(type == "vector_property")
        {
          if(value.contains("object_id") && value.contains("name"))
          {
            const std::string id = value["object_id"];
            if(auto object = Sandbox::getStateData(L).script().world().getObjectByPath(id))
            {
              const std::string name = value["name"];
              if(auto* property = object->getVectorProperty(name); property && property->isScriptReadable())
              {
                return VectorProperty::push(L, *property);
              }
              return lua_pushnil(L);
            }
          }
        }
        else if(type == "method")
        {
          if(value.contains("object_id") && value.contains("name"))
          {
            const std::string id = value["object_id"];
            if(auto object = Sandbox::getStateData(L).script().world().getObjectByPath(id))
            {
              const std::string name = value["name"];
              if(auto* method = object->getMethod(name); method && method->isScriptCallable())
              {
                return Method::push(L, *method);
              }
              return lua_pushnil(L);
            }
          }
        }
        else if(type == "event")
        {
          if(value.contains("object_id") && value.contains("name"))
          {
            const std::string id = value["object_id"];
            if(auto object = Sandbox::getStateData(L).script().world().getObjectByPath(id))
            {
              const std::string name = value["name"];
              if(auto* event = object->getEvent(name); event && event->isScriptable())
              {
                return Event::push(L, *event);
              }
              return lua_pushnil(L);
            }
          }
        }
        else if(type == "pv")
        {
          push(L);
          if(value.contains("items"))
          {
            for(auto item : value["items"])
            {
              if(item.is_object()) /*[[likely]]*/
              {
                push(L, item["key"]);
                push(L, item["value"]);
                lua_settable(L, -3);
              }
            }
          }
          return;
        }
        else if(startsWith(type, "enum."))
        {
          if(value.contains("value"))
          {
            return pushEnum(L, type.substr(5).c_str(), value["value"]);
          }
        }
        else if(startsWith(type, "set."))
        {
          if(value.contains("value"))
          {
            return pushSet(L, type.substr(4).c_str(), value["value"]);
          }
        }
      }
      break;
    }
    case nlohmann::json::value_t::array:
    assert(false);
    case nlohmann::json::value_t::binary:
    case nlohmann::json::value_t::discarded:
      break;
  }
  assert(false);
  errorInternal(L);
}

nlohmann::json PersistentVariables::toJSON(lua_State* L, int index)
{
  switch(lua_type(L, index))
  {
    case LUA_TNIL: /*[[unlikely]]*/
      return nullptr;

    case LUA_TBOOLEAN:
      return (lua_toboolean(L, index) != 0);

    case LUA_TNUMBER:
      if(lua_isinteger(L, index))
      {
        return lua_tointeger(L, index);
      }
      return lua_tonumber(L, index);

    case LUA_TSTRING:
      return lua_tostring(L, index);

    case LUA_TUSERDATA:
      if(auto object = Lua::test<::Object>(L, index))
      {
        auto value = nlohmann::json::object();
        value.emplace("type", "object");
        value.emplace("id", object->getObjectId());
        return value;
      }
      else if(auto* vectorProperty = VectorProperty::test(L, index))
      {
        auto value = nlohmann::json::object();
        value.emplace("type", "vector_property");
        value.emplace("object_id", vectorProperty->object().getObjectId());
        value.emplace("name", vectorProperty->name());
        return value;
      }
      else if(auto* method = Method::test(L, index))
      {
        auto value = nlohmann::json::object();
        value.emplace("type", "method");
        value.emplace("object_id", method->object().getObjectId());
        value.emplace("name", method->name());
        return value;
      }
      else if(auto* event = Event::test(L, index))
      {
        auto value = nlohmann::json::object();
        value.emplace("type", "event");
        value.emplace("object_id", event->object().getObjectId());
        value.emplace("name", event->name());
        return value;
      }
      else if(test(L, index))
      {
        auto items = nlohmann::json::array();

        auto& pv = *static_cast<PersistentVariablesData*>(luaL_checkudata(L, index, metaTableName));
        lua_rawgeti(L, LUA_REGISTRYINDEX, pv.registryIndex);
        assert(lua_istable(L, -1));

        lua_pushnil(L);
        while(lua_next(L, -2))
        {
          auto item = nlohmann::json::object();
          item["key"] = toJSON(L, -2);
          item["value"] = toJSON(L, -1);
          items.push_back(item);
          lua_pop(L, 1); // pop value
        }
        lua_pop(L, 1); // pop table

        auto value = nlohmann::json::object();
        value.emplace("type", "pv");
        value.emplace("items", items);
        return value;
      }
      else if(lua_getmetatable(L, index))
      {
        lua_getfield(L, -1, "__name");
        std::string_view name = lua_tostring(L, -1);
        lua_pop(L, 2);

        if(contains(Enums::metaTableNames, name))
        {
          auto value = nlohmann::json::object();
          value.emplace("type", std::string("enum.").append(name));
          value.emplace("value", checkEnum(L, index, name.data()));
          return value;
        }
        if(contains(Sets::metaTableNames, name))
        {
          auto value = nlohmann::json::object();
          value.emplace("type", std::string("set.").append(name));
          value.emplace("value", checkSet(L, index, name.data()));
          return value;
        }
      }
      break;

    case LUA_TTABLE:
    case LUA_TLIGHTUSERDATA:
    case LUA_TFUNCTION:
    case LUA_TTHREAD:
    default:
      break;
  }
  assert(false);
  errorInternal(L);
}

int PersistentVariables::__index(lua_State* L)
{
  const auto& pv = *static_cast<const PersistentVariablesData*>(lua_touserdata(L, 1));
  lua_rawgeti(L, LUA_REGISTRYINDEX, pv.registryIndex);
  lua_insert(L, 2); // moves key to 3
  lua_rawget(L, 2);
  return 1;
}

int PersistentVariables::__newindex(lua_State* L)
{
  checkValue(L, 2);

  if(lua_istable(L, 3))
  {
    checkTableRecursion(L, 3);

    push(L); // push pv userdata
    lua_insert(L, -2); // swap pv and table
    lua_pushnil(L);
    while(lua_next(L, -2))
    {
      lua_pushvalue(L, -2); // copy key on stack
      lua_insert(L, -2); // swap copied key and value
      lua_settable(L, -5); // pops copied key and value
    }
    lua_pop(L, 1); // pop table
  }
  else if(!test(L, 3))
  {
    checkValue(L, 3);
  }

  const auto& pv = *static_cast<const PersistentVariablesData*>(lua_touserdata(L, 1));
  lua_rawgeti(L, LUA_REGISTRYINDEX, pv.registryIndex);
  lua_insert(L, 2); // moves key to 3 and value to 4
  lua_rawset(L, 2);
  return 0;
}

int PersistentVariables::__pairs(lua_State* L)
{
  lua_getglobal(L, "next");
  assert(lua_isfunction(L, -1));
  const auto& pv = *static_cast<const PersistentVariablesData*>(lua_touserdata(L, 1));
  lua_rawgeti(L, LUA_REGISTRYINDEX, pv.registryIndex);
  assert(lua_istable(L, -1));
  return 2;
}

int PersistentVariables::__len(lua_State* L)
{
  const auto& pv = *static_cast<const PersistentVariablesData*>(lua_touserdata(L, 1));
  lua_rawgeti(L, LUA_REGISTRYINDEX, pv.registryIndex);
  lua_len(L, -1);
  lua_insert(L, -2); // swap length and table
  lua_pop(L, 1); // pop table
  return 1;
}

int PersistentVariables::__gc(lua_State* L)
{
  auto* pv = static_cast<PersistentVariablesData*>(lua_touserdata(L, 1));
  luaL_unref(L, LUA_REGISTRYINDEX, pv->registryIndex);
  pv->~PersistentVariablesData();
  return 0;
}

void PersistentVariables::checkValue(lua_State* L, int index)
{
  switch(lua_type(L, index))
  {
    case LUA_TNIL:
    case LUA_TBOOLEAN:
    case LUA_TNUMBER:
    case LUA_TSTRING:
      return; // supported

    case LUA_TUSERDATA:
      if(Lua::test<::Object>(L, index) || VectorProperty::test(L, index) || Method::test(L, index) || Event::test(L, index))
      {
        return; // supported
      }
      else if(lua_getmetatable(L, index))
      {
        lua_getfield(L, -1, "__name");
        std::string_view name = lua_tostring(L, -1);
        lua_pop(L, 2);

        if(contains(Enums::metaTableNames, name) || contains(Sets::metaTableNames, name))
        {
          return; // supported
        }
      }
      break;

    case LUA_TTABLE:
    case LUA_TLIGHTUSERDATA:
    case LUA_TFUNCTION:
    case LUA_TTHREAD:
    default:
      break;
  }
  errorCantStoreValueAsPersistentVariableUnsupportedType(L);
}

}
