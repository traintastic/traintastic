/**
 * server/src/lua/script.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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
#include "../world/worldloader.hpp"
#include "../world/worldsaver.hpp"
#include "../utils/displayname.hpp"

namespace Lua {

constexpr std::string_view scripts = "scripts";
constexpr std::string_view dotLua = ".lua";

Script::Script(const std::weak_ptr<World>& world, std::string_view _id) :
  IdObject(world, _id),
  m_sandbox{nullptr, nullptr},
  name{this, "name", "", PropertyFlags::ReadWrite | PropertyFlags::Store},
  /*active{this, "active", false, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](bool value)
    {
      if(!value && m_sandbox)
        fini();
      else if(value && !m_sandbox)
        init();
    }},*/
  state{this, "state", LuaScriptState::Stopped, PropertyFlags::ReadOnly | PropertyFlags::Store},
  code{this, "code", "", PropertyFlags::ReadWrite | PropertyFlags::NoStore},
  error{this, "error", "", PropertyFlags::ReadOnly | PropertyFlags::NoStore},
  start{*this, "start",
    [this]()
    {
      if(state != LuaScriptState::Running)
        startSandbox();
    }},
  stop{*this, "stop",
    [this]()
    {
      if(state == LuaScriptState::Running)
        stopSandbox();
    }}
{
  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, false);
  m_interfaceItems.add(name);
  Attributes::addValues(state, LuaScriptStateValues);
  m_interfaceItems.add(state);
  Attributes::addEnabled(code, false);
  m_interfaceItems.add(code);
  m_interfaceItems.add(error);
  Attributes::addEnabled(start, false);
  m_interfaceItems.add(start);
  Attributes::addEnabled(stop, false);
  m_interfaceItems.add(stop);

  updateEnabled();
}

void Script::load(WorldLoader& loader, const nlohmann::json& data)
{
  IdObject::load(loader, data);

  m_basename = id;
  std::string s;
  if(loader.readFile(std::filesystem::path(scripts) / m_basename += dotLua, s))
    code.load(s);
}

void Script::save(WorldSaver& saver, nlohmann::json& data, nlohmann::json& state) const
{
  IdObject::save(saver, data, state);

  if(!m_basename.empty() && m_basename != id.value())
    saver.deleteFile(std::filesystem::path(scripts) / m_basename += dotLua);

  m_basename = id;
  saver.writeFile(std::filesystem::path(scripts) / m_basename += dotLua, code);
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

  updateEnabled();

  if(m_sandbox)
  {
    lua_State* L = m_sandbox.get();
    if(Sandbox::getGlobal(L, "world_event") == LUA_TFUNCTION)
    {
      push(L, state);
      push(L, event);
      pcall(L, 2);
    }
    else
      lua_pop(L, 1);
  }
}

void Script::updateEnabled()
{
  auto w = world().lock();
  const bool editable = w && contains(w->state.value(), WorldState::Edit) && state != LuaScriptState::Running;

  id.setAttributeEnabled(editable);
  name.setAttributeEnabled(editable);
  code.setAttributeEnabled(editable);

  start.setAttributeEnabled(state != LuaScriptState::Running);
  stop.setAttributeEnabled(state == LuaScriptState::Running);
}

void Script::setState(LuaScriptState value)
{
  state.setValueInternal(value);
  updateEnabled();
}

void Script::startSandbox()
{
  assert(!m_sandbox);
  auto world = m_world.lock();
  if(world && (m_sandbox = Sandbox::create(*this)))
  {
    lua_State* L = m_sandbox.get();
    const int r = luaL_loadbuffer(L, code.value().c_str(), code.value().size(), "=") || Sandbox::pcall(L, 0, LUA_MULTRET);
    if(r == LUA_OK)
    {
      if(Sandbox::getGlobal(L, "init") == LUA_TFUNCTION)
        pcall(L);

      setState(LuaScriptState::Running);
      error.setValueInternal("");
    }
    else
    {
      error.setValueInternal(lua_tostring(L, -1));
      setState(LuaScriptState::Error);
      logFatal(error);
      lua_pop(L, 1); // pop error message from the stack
      stopSandbox();
    }
  }
  else
  {
    error.setValueInternal("creating lua state failed");
    setState(LuaScriptState::Error);
    logFatal(error);
  }
}

void Script::stopSandbox()
{
  assert(m_sandbox);
  m_sandbox.reset();
  if(state == LuaScriptState::Running)
    setState(LuaScriptState::Stopped);
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
