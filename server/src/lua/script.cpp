/**
 * server/src/lua/script.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2023 Reinder Feenstra
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
#include "../core/method.tpp"
#include "../core/objectproperty.tpp"
#include "../world/worldloader.hpp"
#include "../world/worldsaver.hpp"
#include "../utils/displayname.hpp"
#include "../log/log.hpp"

namespace Lua {

constexpr std::string_view scripts = "scripts";
constexpr std::string_view dotLua = ".lua";

Script::Script(World& world, std::string_view _id) :
  IdObject(world, _id),
  m_sandbox{nullptr, nullptr},
  name{this, "name", std::string(_id), PropertyFlags::ReadWrite | PropertyFlags::Store},
  disabled{this, "disabled", false, PropertyFlags::ReadWrite | PropertyFlags::NoStore | PropertyFlags::NoScript,
    [this](bool value)
    {
      assert(state != LuaScriptState::Running);
      setState(value ? LuaScriptState::Disabled : LuaScriptState::Stopped);
    }},
  state{this, "state", LuaScriptState::Stopped, PropertyFlags::ReadOnly | PropertyFlags::Store},
  code{this, "code", "", PropertyFlags::ReadWrite | PropertyFlags::NoStore},
  error{this, "error", "", PropertyFlags::ReadOnly | PropertyFlags::NoStore},
  start{*this, "start",
    [this]()
    {
      if(state == LuaScriptState::Stopped || state == LuaScriptState::Error)
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
  Attributes::addEnabled(disabled, false);
  m_interfaceItems.add(disabled);
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
    code.loadJSON(s);
}

void Script::save(WorldSaver& saver, nlohmann::json& data, nlohmann::json& stateData) const
{
  IdObject::save(saver, data, stateData);

  if(!m_basename.empty() && m_basename != id.value())
    saver.deleteFile(std::filesystem::path(scripts) / m_basename += dotLua);

  m_basename = id;
  saver.writeFile(std::filesystem::path(scripts) / m_basename += dotLua, code);
}

void Script::addToWorld()
{
  IdObject::addToWorld();
  m_world.luaScripts->addObject(shared_ptr<Script>());
}

void Script::destroying()
{
  m_world.luaScripts->removeObject(shared_ptr<Script>());
  IdObject::destroying();
}

void Script::loaded()
{
  IdObject::loaded();

  if(state == LuaScriptState::Disabled)
    disabled.setValueInternal(true);
  else if(state == LuaScriptState::Running)
  {
    startSandbox();
    if(state == LuaScriptState::Running)
    {
      auto& running = m_world.luaScripts->status->running;
      running.setValueInternal(running + 1); // setState doesn't increment because the state is already running
    }
  }
}

void Script::worldEvent(WorldState worldState, WorldEvent worldEvent)
{
  IdObject::worldEvent(worldState, worldEvent);

  updateEnabled();
}

void Script::updateEnabled()
{
  const bool editable = contains(m_world.state.value(), WorldState::Edit) && state != LuaScriptState::Running;

  Attributes::setEnabled(id, editable);
  Attributes::setEnabled(name, editable);
  Attributes::setEnabled(disabled, editable);
  Attributes::setEnabled(code, editable);

  Attributes::setEnabled(start, state == LuaScriptState::Stopped || state == LuaScriptState::Error);
  Attributes::setEnabled(stop, state == LuaScriptState::Running);
}

void Script::setState(LuaScriptState value)
{
  const LuaScriptState oldValue = state;
  if(oldValue == value)
    return;

  state.setValueInternal(value);
  updateEnabled();

  // update global lua status:
  {
    auto& status = m_world.luaScripts->status;

    if(oldValue == LuaScriptState::Running) // was running
    {
      assert(status->running != 0);
      status->running.setValueInternal(status->running - 1);
    }
    if(oldValue == LuaScriptState::Error) // was error
    {
      assert(status->error != 0);
      status->error.setValueInternal(status->error - 1);
    }

    if(state == LuaScriptState::Running) // is running
    {
      status->running.setValueInternal(status->running + 1);
    }
    if(state == LuaScriptState::Error) // is error
    {
      status->error.setValueInternal(status->error + 1);
    }
  }
}

void Script::startSandbox()
{
  assert(!m_sandbox);
  if((m_sandbox = Sandbox::create(*this)))
  {
    Log::log(*this, LogMessage::N9001_STARTING_SCRIPT);
    lua_State* L = m_sandbox.get();
    const int r = luaL_loadbuffer(L, code.value().c_str(), code.value().size(), "=") || Sandbox::pcall(L, 0, LUA_MULTRET);
    if(r == LUA_OK)
    {
      setState(LuaScriptState::Running);
      error.setValueInternal("");
    }
    else
    {
      error.setValueInternal(lua_tostring(L, -1));
      setState(LuaScriptState::Error);
      Log::log(*this, LogMessage::F9002_RUNNING_SCRIPT_FAILED_X, error.value());
      lua_pop(L, 1); // pop error message from the stack
      stopSandbox();
    }
  }
  else
  {
    error.setValueInternal("creating lua state failed");
    setState(LuaScriptState::Error);
    Log::log(*this, LogMessage::F9001_CREATING_LUA_STATE_FAILED);
  }
}

void Script::stopSandbox()
{
  assert(m_sandbox);
  m_sandbox.reset();
  if(state == LuaScriptState::Running)
  {
    setState(LuaScriptState::Stopped);
    Log::log(*this, LogMessage::I9001_STOPPED_SCRIPT);
  }
}

bool Script::pcall(lua_State* L, int nargs, int nresults)
{
  const bool success = Sandbox::pcall(L, nargs, nresults) == LUA_OK;
  if(!success)
  {
    Log::log(*this, LogMessage::F9003_CALLING_FUNCTION_FAILED_X, lua_tostring(L, -1));
    lua_pop(L, 1); // pop error message from the stack
  }
  return success;
}

}
