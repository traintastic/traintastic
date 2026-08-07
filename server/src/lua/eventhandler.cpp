/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2021-2026 Reinder Feenstra
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

#include "eventhandler.hpp"
#include "sandbox.hpp"
#include "push.hpp"
#include "to.hpp"
#include "script.hpp"
#include "../core/abstractevent.hpp"
#include "../core/object.hpp"
#include "../log/log.hpp"

namespace Lua {

constexpr char const* eventHandlerGlobal = "event_handlers";

using EventHandlerData = std::weak_ptr<EventHandler>;

EventHandler& EventHandler::check(lua_State* L, int index)
{
  auto& handler = *static_cast<EventHandlerData*>(luaL_checkudata(L, index, metaTableName));
  if(!handler.expired())
  {
    return *handler.lock();
  }
  errorDeadObject(L);
}

EventHandler* EventHandler::test(lua_State* L, int index)
{
  auto* handler = static_cast<EventHandlerData*>(luaL_testudata(L, index, metaTableName));
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

void EventHandler::push(lua_State* L, EventHandler& value)
{
  lua_getglobal(L, eventHandlerGlobal);
  lua_rawgetp(L, -1, &value);
  if(lua_isnil(L, -1)) // method not in table
  {
    lua_pop(L, 1); // remove nil
    new(lua_newuserdata(L, sizeof(EventHandlerData))) EventHandlerData(std::static_pointer_cast<EventHandler>(value.shared_from_this()));
    luaL_setmetatable(L, metaTableName);
    lua_pushvalue(L, -1); // copy userdata on stack
    lua_rawsetp(L, -3, &value); // add method to table
  }
  lua_insert(L, lua_gettop(L) - 1); // swap table and userdata
  lua_pop(L, 1); // remove table
}

void EventHandler::registerType(lua_State* L)
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
  lua_setglobal(L, eventHandlerGlobal);
}

int EventHandler::__gc(lua_State* L)
{
  static_cast<EventHandlerData*>(lua_touserdata(L, 1))->~EventHandlerData();
  return 0;
}

int EventHandler::__index(lua_State* L)
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

int EventHandler::disconnect(lua_State* L)
{
  check(L, lua_upvalueindex(1)).disconnect();
  return 0;
}

int EventHandler::pause(lua_State* L)
{
  check(L, lua_upvalueindex(1)).m_paused = true;
  return 0;
}

int EventHandler::resume(lua_State* L)
{
  check(L, lua_upvalueindex(1)).m_paused = false;
  return 0;
}


EventHandler::EventHandler(AbstractEvent& evt, lua_State* L, int functionIndex)
  : AbstractEventHandler(evt)
  , m_L{L}
  , m_function{LUA_NOREF}
  , m_userData{LUA_NOREF}
{
  luaL_checktype(L, functionIndex, LUA_TFUNCTION);

  // add function to registry:
  lua_pushvalue(L, functionIndex);
  m_function = luaL_ref(m_L, LUA_REGISTRYINDEX);

  // add userdata to registry (if available):
  if(!lua_isnoneornil(L, functionIndex + 1))
  {
    lua_pushvalue(L, functionIndex + 1);
    m_userData = luaL_ref(m_L, LUA_REGISTRYINDEX);;
  }
}

EventHandler::~EventHandler()
{
  release();
}

void EventHandler::execute(const Arguments& args)
{
  if(m_paused)
  {
    return;
  }

  const auto argumentTypeInfo = m_event.argumentTypeInfo();
  assert(args.size() == argumentTypeInfo.size());

  if(args.size() != argumentTypeInfo.size())
    return;

  lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_function);

  const size_t nargs = args.size();
  for(size_t i = 0; i < nargs; i++)
  {
    const auto& arg = args[i];
    switch(argumentTypeInfo[i].type)
    {
      case ValueType::Boolean:
        Lua::push(m_L, std::get<bool>(arg));
        break;

      case ValueType::Enum:
        pushEnum(m_L, argumentTypeInfo[i].enumName.data(), std::get<int64_t>(arg));
        assert(lua_isuserdata(m_L, -1)); // check if enum value is known
        break;

      case ValueType::Integer:
        Lua::push(m_L, std::get<int64_t>(arg));
        break;

      case ValueType::Float:
        Lua::push(m_L, std::get<double>(arg));
        break;

      case ValueType::String:
        Lua::push(m_L, std::get<std::string>(arg));
        break;

      case ValueType::Object:
        Lua::push(m_L, std::get<ObjectPtr>(arg));
        break;

      case ValueType::Set:
        pushSet(m_L, argumentTypeInfo[i].setName.data(), std::get<int64_t>(arg));
        break;

      case ValueType::Invalid:
      default:
        assert(false);
        lua_pushnil(m_L);
        break;
    }
  }

  lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_userData);

  if(Sandbox::pcall(m_L, args.size() + 1, 0, 0) != LUA_OK)
  {
    Log::log(
      Sandbox::getStateData(m_L).script().id,
      LogMessage::E9001_X_DURING_EXECUTION_OF_X_EVENT_HANDLER,
      to<std::string_view>(m_L, -1),
      m_event.object().getObjectId().append(".").append(m_event.name()));
  }
}

bool EventHandler::disconnect()
{
  Sandbox::getStateData(m_L).unregisterEventHandler(std::dynamic_pointer_cast<EventHandler>(shared_from_this()));
  release();
  return AbstractEventHandler::disconnect();
}

void EventHandler::release()
{
  if(m_L)
  {
    luaL_unref(m_L, LUA_REGISTRYINDEX, m_function);
    luaL_unref(m_L, LUA_REGISTRYINDEX, m_userData);
    m_L = nullptr;
  }
}

}
