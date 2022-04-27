/**
 * server/src/lua/eventhandler.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_LUA_EVENTHANDLER_HPP
#define TRAINTASTIC_SERVER_LUA_EVENTHANDLER_HPP

#include <lua.hpp>
#include "../core/abstracteventhandler.hpp"

namespace Lua {

class EventHandler final : public AbstractEventHandler
{
  private:
    lua_State* m_L;
    int m_function;
    int m_userData;

    void release();

  public:
    EventHandler(AbstractEvent& evt, lua_State* L, int functionIndex = 1);
    ~EventHandler() final;

    void execute(const Arguments& args) final;

    bool disconnect() final;
};

}

#endif
