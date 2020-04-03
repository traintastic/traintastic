/**
 * server/src/lua/sandbox.hpp - Lua sandbox
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

#ifndef TRAINTASTIC_SERVER_LUA_SANDBOX_HPP
#define TRAINTASTIC_SERVER_LUA_SANDBOX_HPP

#include <memory>
#include <lua.hpp>

namespace Lua {

class Script;

using SandboxPtr = std::unique_ptr<lua_State, void(*)(lua_State*)>;

class Sandbox
{
  private:
    static void close(lua_State* L);

  public:
    class StateData
    {
      private:
        Script& m_script;

      public:
        StateData(Script& script) :
          m_script{script}
        {
        }

        inline Script& script() const
        {
          return m_script;
        }
    };

    static SandboxPtr create(Script& script);
    static StateData& getStateData(lua_State* L);
    static int getGlobal(lua_State* L, const char* name);
    static int pcall(lua_State* L, int nargs = 0, int nresults = 0, int errfunc = 0);
};

}

#endif
