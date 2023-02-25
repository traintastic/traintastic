/**
 * server/src/lua/scriptlist.hpp
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

#ifndef TRAINTASTIC_SERVER_LUA_SCRIPTLIST_HPP
#define TRAINTASTIC_SERVER_LUA_SCRIPTLIST_HPP

#include "../core/objectlist.hpp"
#include "../core/method.hpp"
#include "script.hpp"

namespace Lua {

class ScriptList : public ObjectList<Script>
{
  protected:
    void worldEvent(WorldState state, WorldEvent event) final;
    bool isListedProperty(std::string_view name) final;

  public:
    CLASS_ID("lua.script_list")

    ::Method<std::shared_ptr<Script>()> create;
    ::Method<void(const std::shared_ptr<Script>&)> delete_;
    ::Method<void()> startAll;
    ::Method<void()> stopAll;

    ScriptList(Object& _parent, std::string_view parentPropertyName);

    TableModelPtr getModel() final;
};

}

#endif
