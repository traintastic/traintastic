/**
 * server/src/lua/script.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_LUA_SCRIPT_HPP
#define TRAINTASTIC_SERVER_LUA_SCRIPT_HPP

#include "../core/idobject.hpp"
#include "../core/method.hpp"
#include "../enum/luascriptstate.hpp"
#include "sandbox.hpp"

namespace Lua {

class Script : public IdObject
{
  friend class Sandbox;

  private:
    mutable std::string m_basename; //!< filename on disk for script

  protected:
    SandboxPtr m_sandbox;
    nlohmann::json m_persistentVariables;

    void load(WorldLoader& loader, const nlohmann::json& data) final;
    void save(WorldSaver& saver, nlohmann::json& data, nlohmann::json& stateData) const final;
    void addToWorld() final;
    void destroying() final;
    void loaded() final;
    void worldEvent(WorldState worldState, WorldEvent worldEvent) final;

    void updateEnabled();
    void setState(LuaScriptState value);

    void startSandbox();
    void stopSandbox();
    bool pcall(lua_State* L, int nargs = 0, int nresults = 0);

  public:
    CLASS_ID("lua.script")
    CREATE(Script)

    Script(World& world, std::string_view _id);

    Property<std::string> name;
    Property<bool> disabled;
    Property<LuaScriptState> state;
    Property<std::string> code;
    Property<std::string> error;
    ::Method<void()> start;
    ::Method<void()> stop;
    ::Method<void()> clearPersistentVariables;
};

}

#endif
