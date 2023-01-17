/**
 * server/src/lua/scriptlist.cpp
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

#include "scriptlist.hpp"
#include "scriptlisttablemodel.hpp"
#include "../world/getworld.hpp"
#include "../core/attributes.hpp"
#include "../utils/displayname.hpp"

namespace Lua {

ScriptList::ScriptList(Object& _parent, std::string_view parentPropertyName) :
  ObjectList<Script>(_parent, parentPropertyName),
  add{*this, "add",
    [this]()
    {
      auto& world = getWorld(parent());
      return Script::create(world, world.getUniqueId("script"));
    }}
  , remove{*this, "remove", std::bind(&ScriptList::removeMethodHandler, this, std::placeholders::_1)}
  , startAll{*this, "start_all",
      [this]()
      {
        for(const auto& script : m_items)
          if(!script->disabled)
            script->start();
      }}
  , stopAll{*this, "stop_all",
      [this]()
      {
        for(const auto& script : m_items)
          if(!script->disabled)
            script->stop();
      }}
{
  const bool editable = contains(getWorld(parent()).state.value(), WorldState::Edit);

  Attributes::addDisplayName(add, DisplayName::List::add);
  Attributes::addEnabled(add, editable);
  m_interfaceItems.add(add);

  Attributes::addDisplayName(remove, DisplayName::List::remove);
  Attributes::addEnabled(remove, editable);
  m_interfaceItems.add(remove);

  m_interfaceItems.add(startAll);

  m_interfaceItems.add(stopAll);
}

TableModelPtr ScriptList::getModel()
{
  return std::make_shared<ScriptListTableModel>(*this);
}

void ScriptList::worldEvent(WorldState state, WorldEvent event)
{
  ObjectList<Script>::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(add, editable);
}

bool ScriptList::isListedProperty(std::string_view name)
{
  return ScriptListTableModel::isListedProperty(name);
}

}
