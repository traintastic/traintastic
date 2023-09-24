/**
 * server/src/lua/eventhandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
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
        push(m_L, std::get<bool>(arg));
        break;

      case ValueType::Enum:
        pushEnum(m_L, argumentTypeInfo[i].enumName.data(), std::get<int64_t>(arg));
        assert(lua_isuserdata(m_L, -1)); // check if enum value is known
        break;

      case ValueType::Integer:
        push(m_L, std::get<int64_t>(arg));
        break;

      case ValueType::Float:
        push(m_L, std::get<double>(arg));
        break;

      case ValueType::String:
        push(m_L, std::get<std::string>(arg));
        break;

      case ValueType::Object:
        push(m_L, std::get<ObjectPtr>(arg));
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
