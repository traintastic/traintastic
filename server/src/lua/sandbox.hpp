/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2019-2026 Reinder Feenstra
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
#include <set>
#include <algorithm>
#include <limits>
#include <chrono>
#include <cassert>
#include <vector>
#include <lua.hpp>

class InputController;
class Input;
class OutputController;
class Output;
class ScriptThrottle;

namespace Lua {

class Script;
class EventHandler;
class OnChangedHandler;

using SandboxPtr = std::unique_ptr<lua_State, void(*)(lua_State*)>;

class Sandbox
{
  private:
    static constexpr auto pcallDurationMax = std::chrono::milliseconds(10); //!< Execution time limit
    static constexpr auto pcallDurationWarning = pcallDurationMax / 2; //!< Execution time warning level

    static void close(lua_State* L);
    static int __index(lua_State* L);
    static int __newindex(lua_State* L);

    static void* alloc(void* ud, void* ptr, size_t osize, size_t nsize);
    static void hook(lua_State* L, lua_Debug* /*ar*/);

  public:
    class StateData
    {
      private:
        Script& m_script;
        lua_Integer m_eventHandlerId;
        std::vector<std::shared_ptr<EventHandler>> m_eventHandlers;
        std::vector<std::shared_ptr<OnChangedHandler>> m_onChangedHandlers;
        std::map<
          std::weak_ptr<InputController>,
          std::set<std::weak_ptr<Input>, std::owner_less<std::weak_ptr<Input>>>,
          std::owner_less<std::weak_ptr<InputController>>
          > m_inputs;
        std::map<
          std::weak_ptr<OutputController>,
          std::set<std::weak_ptr<Output>, std::owner_less<std::weak_ptr<Output>>>,
          std::owner_less<std::weak_ptr<OutputController>>
          > m_outputs;
        std::vector<std::shared_ptr<ScriptThrottle>> m_throttles;

      public:
        static constexpr size_t memoryLimit = 1024 * 1024; // 1 MiB
        size_t memoryUsed = 0;
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

        void registerEventHandler(std::shared_ptr<EventHandler> handler);
        void unregisterEventHandler(const std::shared_ptr<EventHandler>& handler);

        void registerOnChangedHandler(std::shared_ptr<OnChangedHandler> handler);
        void unregisterOnChangedHandler(const std::shared_ptr<OnChangedHandler>& handler);

        void registerInput(std::weak_ptr<InputController> inputController, std::weak_ptr<Input> input)
        {
          m_inputs[inputController].emplace(input);
        }

        void registerOutput(std::weak_ptr<OutputController> outputController, std::weak_ptr<Output> output)
        {
          m_outputs[outputController].emplace(output);
        }

        void addThrottle(std::shared_ptr<ScriptThrottle> throttle)
        {
          m_throttles.emplace_back(std::move(throttle));
        }
    };

    static SandboxPtr create(Script& script);
    static StateData& getStateData(lua_State* L);
    static int getGlobal(lua_State* L, const char* name);
    static int pcall(lua_State* L, int nargs = 0, int nresults = 0, int errfunc = 0);
    static void syncPersistentVariables(lua_State* L);
};

}

#endif
