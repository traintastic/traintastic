/**
 * server/src/lua/script.cpp
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

#include "script.hpp"
#include "scriptlist.hpp"
#include "scriptlisttablemodel.hpp"
#include "push.hpp"
#include "../world/world.hpp"
#include "../enum/worldevent.hpp"
#include "../set/worldstate.hpp"
#include "../core/attributes.hpp"

namespace Lua {

Script::Script(const std::weak_ptr<World>& world, std::string_view _id) :
  IdObject(world, _id),
  m_sandbox{nullptr, nullptr},
  name{this, "name", "", PropertyFlags::ReadWrite | PropertyFlags::Store},
  active{this, "active", false, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](bool value)
    {
      if(!value && m_sandbox)
        fini();
      else if(value && !m_sandbox)
        init();
    }},
  code{this, "code", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  auto w = world.lock();
  const bool editable = w && contains(w->state.value(), WorldState::Edit);

  m_interfaceItems.add(name);
  m_interfaceItems.add(active);
  Attributes::addEnabled(code, !active && editable);
  m_interfaceItems.add(code);
}

void Script::addToWorld()
{
  IdObject::addToWorld();

  if(auto world = m_world.lock())
    world->luaScripts->addObject(shared_ptr<Script>());
}

void Script::worldEvent(WorldState state, WorldEvent event)
{
  IdObject::worldEvent(state, event);

  code.setAttributeEnabled(!active && contains(state, WorldState::Edit));

  if(active && m_sandbox)
  {
    lua_State* L = m_sandbox.get();
    if(Sandbox::getGlobal(L, "world_event") == LUA_TFUNCTION)
    {
      push(L, state);
      push(L, event);
      pcall(L, 2);
    }
  }
}

void Script::init()
{
  assert(!m_sandbox);
  auto world = m_world.lock();
  if(world && (m_sandbox = Sandbox::create(*this)))
  {
    lua_State* L = m_sandbox.get();
    const int error = luaL_loadbuffer(L, code.value().c_str(), code.value().size(), "=") || Sandbox::pcall(L, 0, LUA_MULTRET);
    if(error == LUA_OK)
    {
      if(Sandbox::getGlobal(L, "init") == LUA_TFUNCTION)
        pcall(L);
    }
    else
    {
      logFatal(lua_tostring(L, -1));
      lua_pop(L, 1); // pop error message from the stack
      fini();
    }
  }
  else
    logFatal("Creating lua state failed");
}

void Script::fini()
{
  assert(m_sandbox);
  m_sandbox.reset();
}

bool Script::pcall(lua_State* L, int nargs, int nresults)
{
  const bool success = Sandbox::pcall(L, nargs, nresults) == LUA_OK;
  if(!success)
  {
    logCritical(lua_tostring(L, -1));
    lua_pop(L, 1); // pop error message from the stack
  }
  return success;
}

}
