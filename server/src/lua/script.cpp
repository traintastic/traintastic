/**
 * server/src/lua/script.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020 Reinder Feenstra
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
#include "push.hpp"
#include "../core/traintastic.hpp"
#include <enum/traintasticmode.hpp>

namespace Lua {

Script::Script(const std::weak_ptr<World>& world, const std::string& _id) :
  IdObject(world, _id),
  m_sandbox{nullptr, nullptr},
  name{this, "name", "", PropertyFlags::ReadWrite | PropertyFlags::Store},
  enabled{this, "enabled", false, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](bool value)
    {
      if(!value && m_sandbox)
      {
        assert(Traintastic::instance->mode != TraintasticMode::Run);
        fini();
      }
      else if(value && Traintastic::instance->mode == TraintasticMode::Stop)
      {
        assert(!m_sandbox);
        init();  
      }
    }},
  code{this, "code", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  m_interfaceItems.add(name);
  m_interfaceItems.add(enabled)
    .addAttributeEnabled(false);
  m_interfaceItems.add(code)
    .addAttributeEnabled(false);
}

void Script::modeChanged(TraintasticMode mode)
{
  IdObject::modeChanged(mode);

  enabled.setAttributeEnabled(mode != TraintasticMode::Run);
  code.setAttributeEnabled(mode == TraintasticMode::Edit);

  if(enabled)
  {
    if(mode == TraintasticMode::Edit && m_sandbox)
      fini();
    else if(mode == TraintasticMode::Stop && !m_sandbox)
      init();
    else if(m_sandbox)
    {
      lua_State* L = m_sandbox.get();
      if(Sandbox::getGlobal(L, "mode_changed") == LUA_TFUNCTION)
      {
        push(L, mode);
        pcall(L, 1);
      }     
    }
  } 
}

void Script::init()
{
  assert(!m_sandbox);
  if((m_sandbox = Sandbox::create()))
  {
    lua_State* L = m_sandbox.get();
    const int error = luaL_loadbuffer(L, code.value().c_str(), code.value().size(), "=") || Sandbox::pcall(L, 0, LUA_MULTRET);
    if(error == 0)
    {
      if(Sandbox::getGlobal(L, "init") == LUA_TFUNCTION)
        pcall(L);
    }
    else
    {
      Traintastic::instance->console->fatal(id, lua_tostring(L, -1));
      lua_pop(L, 1); // pop error message from the stack
      fini();
    }
  }
  else
    Traintastic::instance->console->fatal(id, "Creating lua state failed");
}

void Script::fini()
{
  assert(m_sandbox);
  m_sandbox.reset();
}

bool Script::pcall(lua_State* L, int nargs, int nresults)
{
  const bool success = Sandbox::pcall(L, nargs, nresults) == 0;
  if(!success)
  {
    Traintastic::instance->console->critical(id, lua_tostring(L, -1));
    lua_pop(L, 1); // pop error message from the stack
  }
  return success;
}





}
