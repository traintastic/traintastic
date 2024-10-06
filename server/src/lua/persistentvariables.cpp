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
  lua_pushcfunction(L, __gc);
  lua_setfield(L, -2, "__gc");
  lua_pop(L, 1);
}

void PersistentVariables::push(lua_State* L)
{
  auto* pv = static_cast<PersistentVariablesData*>(lua_newuserdatauv(L, sizeof(PersistentVariablesData), 1));
  luaL_getmetatable(L, metaTableName);
  lua_setmetatable(L, -2);
  lua_newtable(L);
  pv->registryIndex = luaL_ref(L, LUA_REGISTRYINDEX);
}

void PersistentVariables::push(lua_State* L, const nlohmann::json& pv)
{
  push(L);

  if(!pv.is_object())
  {
    return;
  }

  for(auto item : pv.items())
  {
    switch(item.value().type())
    {
      case nlohmann::json::value_t::null:
        continue; // no need to set nil value

      case nlohmann::json::value_t::boolean:
        lua_pushboolean(L, static_cast<bool>(item.value()));
        break;

      case nlohmann::json::value_t::number_integer:
      case nlohmann::json::value_t::number_unsigned:
        lua_pushinteger(L, item.value());
        break;

      case nlohmann::json::value_t::number_float:
        lua_pushnumber(L, item.value());
        break;

      case nlohmann::json::value_t::string:
      {
        const std::string s = item.value();
        lua_pushlstring(L, s.data(), s.size());
        break;
      }
      case nlohmann::json::value_t::object:
      {
        const auto& value = item.value();
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
                Object::push(L, object);
                break;
              }
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
                  VectorProperty::push(L, *property);
                  break;
                }
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
                  Method::push(L, *method);
                  break;
                }
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
                  Event::push(L, *event);
                  break;
                }
              }
            }
          }
          else if(startsWith(type, "enum."))
          {
            if(value.contains("value"))
            {
              pushEnum(L, type.substr(5).c_str(), value["value"]);
              break;
            }
          }
          else if(startsWith(type, "set."))
          {
            if(value.contains("value"))
            {
              pushSet(L, type.substr(4).c_str(), value["value"]);
              break;
            }
          }
        }
        continue;
      }
      case nlohmann::json::value_t::array:
      case nlohmann::json::value_t::binary:
      case nlohmann::json::value_t::discarded:
        assert(false);
        continue;
    }
    lua_setfield(L, -2, item.key().c_str());
  }
}

nlohmann::json PersistentVariables::toJSON(lua_State* L, int index)
{
  auto result = nlohmann::json::object();

  auto& pv = *static_cast<PersistentVariablesData*>(luaL_checkudata(L, index, metaTableName));
  lua_rawgeti(L, LUA_REGISTRYINDEX, pv.registryIndex);
  assert(lua_istable(L, -1));

  lua_pushnil(L);
  while(lua_next(L, -2))
  {
    const char* key = lua_tostring(L, -2);
    switch(lua_type(L, -1))
    {
      case LUA_TNIL: /*[[unlikely]]*/
        result.emplace(key, nullptr);
        break;

      case LUA_TBOOLEAN:
        result.emplace(key, lua_toboolean(L, -1) != 0);
        break;

      case LUA_TNUMBER:
        if(lua_isinteger(L, -1))
        {
          result.emplace(key, lua_tointeger(L, -1));
        }
        else
        {
          result.emplace(key, lua_tonumber(L, -1));
        }
        break;

      case LUA_TSTRING:
        result.emplace(key, std::string(lua_tostring(L, -1)));
        break;

      case LUA_TUSERDATA:
        if(auto object = test<::Object>(L, -1))
        {
          auto value = nlohmann::json::object();
          value.emplace("type", "object");
          value.emplace("id", object->getObjectId());
          result.emplace(key, std::move(value));
          break;
        }
        else if(auto* vectorProperty = VectorProperty::test(L, -1))
        {
          auto value = nlohmann::json::object();
          value.emplace("type", "vector_property");
          value.emplace("object_id", vectorProperty->object().getObjectId());
          value.emplace("name", vectorProperty->name());
          result.emplace(key, std::move(value));
          break;
        }
        else if(auto* method = Method::test(L, -1))
        {
          auto value = nlohmann::json::object();
          value.emplace("type", "method");
          value.emplace("object_id", method->object().getObjectId());
          value.emplace("name", method->name());
          result.emplace(key, std::move(value));
          break;
        }
        else if(auto* event = Event::test(L, -1))
        {
          auto value = nlohmann::json::object();
          value.emplace("type", "event");
          value.emplace("object_id", event->object().getObjectId());
          value.emplace("name", event->name());
          result.emplace(key, std::move(value));
          break;
        }
        else if(lua_getmetatable(L, -1))
        {
          lua_getfield(L, -1, "__name");
          std::string_view name = lua_tostring(L, -1);
          lua_pop(L, 2);

          if(contains(Enums::metaTableNames, name))
          {
            auto value = nlohmann::json::object();
            value.emplace("type", std::string("enum.").append(name));
            value.emplace("value", checkEnum(L, -1, name.data()));
            result.emplace(key, std::move(value));
            break;
          }
          else if(contains(Sets::metaTableNames, name))
          {
            auto value = nlohmann::json::object();
            value.emplace("type", std::string("set.").append(name));
            value.emplace("value", checkSet(L, -1, name.data()));
            result.emplace(key, std::move(value));
            break;
          }
        }
        assert(false);
        break;

      case LUA_TLIGHTUSERDATA:
      case LUA_TTABLE:
      case LUA_TFUNCTION:
      case LUA_TTHREAD:
      default:
        assert(false); // unsupported
        break;
    }
    lua_pop(L, 1); // pop value
  }

  return result;
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
  switch(lua_type(L, 3))
  {
    case LUA_TNIL:
    case LUA_TBOOLEAN:
    case LUA_TNUMBER:
    case LUA_TSTRING:
      break; // supported

    case LUA_TUSERDATA:
      if(test<::Object>(L, 3) || VectorProperty::test(L, 3) || Method::test(L, 3) || Event::test(L, 3))
      {
        break; // supported
      }
      else if(lua_getmetatable(L, 3))
      {
        lua_getfield(L, -1, "__name");
        std::string_view name = lua_tostring(L, -1);
        lua_pop(L, 2);

        if(contains(Enums::metaTableNames, name) || contains(Sets::metaTableNames, name))
        {
          break; // supported
        }
      }
      errorCantStoreValueAsPersistentVariableUnsupportedType(L);

    case LUA_TLIGHTUSERDATA:
    case LUA_TTABLE:
    case LUA_TFUNCTION:
    case LUA_TTHREAD:
    default:
      errorCantStoreValueAsPersistentVariableUnsupportedType(L);
  }

  const auto& pv = *static_cast<const PersistentVariablesData*>(lua_touserdata(L, 1));
  lua_rawgeti(L, LUA_REGISTRYINDEX, pv.registryIndex);
  lua_insert(L, 2); // moves key to 3 and value to 4
  lua_rawset(L, 2);
  return 0;
}

int PersistentVariables::__gc(lua_State* L)
{
  auto* pv = static_cast<PersistentVariablesData*>(lua_touserdata(L, 1));
  luaL_unref(L, LUA_REGISTRYINDEX, pv->registryIndex);
  pv->~PersistentVariablesData();
  return 0;
}

}
