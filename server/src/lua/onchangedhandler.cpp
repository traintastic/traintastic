/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#include "onchangedhandler.hpp"
#include "object/object.hpp"
#include "push.hpp"
#include "sandbox.hpp"
#include "script.hpp"
#include "to.hpp"
#include "vectorproperty.hpp"
#include "../core/abstractproperty.hpp"
#include "../core/abstractvectorproperty.hpp"
#include "../core/object.hpp"
#include "../log/log.hpp"
#include "../utils/contains.hpp"

namespace Lua {

constexpr char const* onChangedGlobal = "on_changeds";

using OnChangedData = std::weak_ptr<OnChangedHandler>;

OnChangedHandler& OnChangedHandler::check(lua_State* L, int index)
{
  auto& handler = *static_cast<OnChangedData*>(luaL_checkudata(L, index, metaTableName));
  if(!handler.expired())
  {
    return *handler.lock();
  }
  errorDeadObject(L);
}

OnChangedHandler* OnChangedHandler::test(lua_State* L, int index)
{
  auto* handler = static_cast<OnChangedData*>(luaL_testudata(L, index, metaTableName));
  if(!handler)
  {
    return nullptr;
  }
  if(!handler->expired())
  {
    return handler->lock().get();
  }
  errorDeadObject(L);
}

void OnChangedHandler::push(lua_State* L, OnChangedHandler& value)
{
  lua_getglobal(L, onChangedGlobal);
  lua_rawgetp(L, -1, &value);
  if(lua_isnil(L, -1)) // method not in table
  {
    lua_pop(L, 1); // remove nil
    new(lua_newuserdata(L, sizeof(OnChangedData))) OnChangedData(value.shared_from_this());
    luaL_setmetatable(L, metaTableName);
    lua_pushvalue(L, -1); // copy userdata on stack
    lua_rawsetp(L, -3, &value); // add method to table
  }
  lua_insert(L, lua_gettop(L) - 1); // swap table and userdata
  lua_pop(L, 1); // remove table
}

void OnChangedHandler::registerType(lua_State* L)
{
  luaL_newmetatable(L, metaTableName);
  lua_pushcfunction(L, __gc);
  lua_setfield(L, -2, "__gc");
  lua_pushcfunction(L, __index);
  lua_setfield(L, -2, "__index");
  lua_pop(L, 1);

  // weak table for on_changed userdata:
  lua_newtable(L);
  lua_newtable(L); // metatable
  lua_pushliteral(L, "__mode");
  lua_pushliteral(L, "v");
  lua_rawset(L, -3);
  lua_setmetatable(L, -2);
  lua_setglobal(L, onChangedGlobal);
}

int OnChangedHandler::__gc(lua_State* L)
{
  static_cast<OnChangedData*>(lua_touserdata(L, 1))->~OnChangedData();
  return 0;
}

int OnChangedHandler::__index(lua_State* L)
{
  auto& handler = check(L, 1);
  const auto key = to<std::string_view>(L, 2);

  if(key == "disconnect")
  {
    push(L, handler);
    lua_pushcclosure(L, disconnect, 1);
    return 1;
  }
  if(key == "pause")
  {
    push(L, handler);
    lua_pushcclosure(L, pause, 1);
    return 1;
  }
  if(key == "resume")
  {
    push(L, handler);
    lua_pushcclosure(L, resume, 1);
    return 1;
  }
  return 0;
}

int OnChangedHandler::disconnect(lua_State* L)
{
  check(L, lua_upvalueindex(1)).disconnect();
  return 0;
}

int OnChangedHandler::pause(lua_State* L)
{
  check(L, lua_upvalueindex(1)).m_paused = true;
  return 0;
}

int OnChangedHandler::resume(lua_State* L)
{
  check(L, lua_upvalueindex(1)).m_paused = false;
  return 0;
}


OnChangedHandler::OnChangedHandler(::Object& object, lua_State* L, int functionIndex)
  : m_L{L}
  , m_function{LUA_NOREF}
  , m_userData{LUA_NOREF}
  , m_connection{object.propertyChanged.connect(std::bind_front(&OnChangedHandler::propertyChanged, this))}
{
  luaL_checktype(L, functionIndex, LUA_TFUNCTION);

  // add function to registry:
  lua_pushvalue(L, functionIndex);
  m_function = luaL_ref(m_L, LUA_REGISTRYINDEX);

  // add userdata to registry (if available):
  if(!lua_isnoneornil(L, functionIndex + 1))
  {
    lua_pushvalue(L, functionIndex + 1);
    m_userData = luaL_ref(m_L, LUA_REGISTRYINDEX);
  }
}

OnChangedHandler::~OnChangedHandler()
{
  release();
}

void OnChangedHandler::disconnect()
{
  m_connection.disconnect();
  Sandbox::getStateData(m_L).unregisterOnChangedHandler(std::dynamic_pointer_cast<OnChangedHandler>(shared_from_this()));
  release();
}

void OnChangedHandler::setFilter(std::vector<std::string> filter)
{
  m_filter = std::move(filter);
}

void OnChangedHandler::propertyChanged(::BaseProperty& baseProperty)
{
  if(m_paused)
  {
    return;
  }
  if(!baseProperty.isScriptReadable())
  {
    return;
  }
  if(!m_filter.empty() && !contains(m_filter, baseProperty.name()))
  {
    return;
  }

  // build stack:
  // - function
  // - value
  // - object
  // - name
  // - user_data

  lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_function);

  if(auto* property = dynamic_cast<AbstractProperty*>(&baseProperty))
  {
    Object::Object::pushPropertyValue(m_L, *property);
  }
  else if(auto* vectorProperty = dynamic_cast<AbstractVectorProperty*>(&baseProperty))
  {
    VectorProperty::push(m_L, *vectorProperty);
  }
  else
  {
    assert(false);
    lua_pop(m_L, 1); // remove function from stack
    return;
  }

  Lua::Object::push(m_L, baseProperty.object().shared_from_this());
  Lua::push(m_L, baseProperty.name());
  lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_userData);

  if(Sandbox::pcall(m_L, 4, 0, 0) != LUA_OK)
  {
    Log::log(
      Sandbox::getStateData(m_L).script().id,
      LogMessage::E9002_X_DURING_EXECUTION_OF_X_ON_CHANGED_HANDLER,
      to<std::string_view>(m_L, -1),
      baseProperty.object().getObjectId().append(".").append(baseProperty.name()));
  }
}

void OnChangedHandler::release()
{
  if(m_L)
  {
    luaL_unref(m_L, LUA_REGISTRYINDEX, m_function);
    luaL_unref(m_L, LUA_REGISTRYINDEX, m_userData);
    m_L = nullptr;
  }
}

}
