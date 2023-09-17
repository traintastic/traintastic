/**
 * server/src/lua/sandbox.hpp - Lua sandbox
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2022-2023 Reinder Feenstra
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
#include <map>
#include <algorithm>
#include <limits>
#include <chrono>
#include <cassert>
#include <lua.hpp>

namespace Lua {

class Script;
class EventHandler;

using SandboxPtr = std::unique_ptr<lua_State, void(*)(lua_State*)>;

class Sandbox
{
  private:
    static constexpr auto pcallDurationMax = std::chrono::milliseconds(10); //!< Execution time limit
    static constexpr auto pcallDurationWarning = pcallDurationMax / 2; //!< Execution time warning level

    static void close(lua_State* L);
    static int __index(lua_State* L);
    static int __newindex(lua_State* L);

    static void hook(lua_State* L, lua_Debug* /*ar*/);

  public:
    class StateData
    {
      private:
        Script& m_script;
        lua_Integer m_eventHandlerId;
        std::map<lua_Integer, std::shared_ptr<EventHandler>> m_eventHandlers;

      public:
        std::chrono::time_point<std::chrono::steady_clock> pcallStart;
        bool pcallExecutionTimeViolation;

        StateData(Script& script)
          : m_script{script}
          , m_eventHandlerId{1}
        {
        }

        ~StateData();

        inline Script& script() const
        {
          return m_script;
        }

        std::shared_ptr<EventHandler> getEventHandler(lua_Integer id) const
        {
          auto it = m_eventHandlers.find(id);
          if(it != m_eventHandlers.end())
            return it->second;
          else
            return std::shared_ptr<EventHandler>();
        }

        inline lua_Integer registerEventHandler(std::shared_ptr<EventHandler> handler)
        {
          while(m_eventHandlers.find(m_eventHandlerId) != m_eventHandlers.end())
          {
            if(m_eventHandlerId == std::numeric_limits<lua_Integer>::max())
              m_eventHandlerId = 1;
            else
              m_eventHandlerId++;
          }
          const lua_Integer id = m_eventHandlerId;
          m_eventHandlerId++;
          m_eventHandlers.emplace(id, std::move(handler));
          return id;
        }

        inline void unregisterEventHandler(const std::shared_ptr<EventHandler>& handler)
        {
          auto it = std::find_if(m_eventHandlers.begin(), m_eventHandlers.end(),
            [&handler](const auto& elem)
            {
              return elem.second == handler;
            });

          if(it != m_eventHandlers.end())
            m_eventHandlers.erase(it);
        }
    };

    static SandboxPtr create(Script& script);
    static StateData& getStateData(lua_State* L);
    static int getGlobal(lua_State* L, const char* name);
    static int pcall(lua_State* L, int nargs = 0, int nresults = 0, int errfunc = 0);
};

}

#endif
