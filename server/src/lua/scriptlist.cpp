/**
 * server/src/lua/scriptlist.cpp
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

#include "scriptlist.hpp"
#include "scriptlisttablemodel.hpp"
#include "../world/getworld.hpp"
#include "../core/attributes.hpp"
#include "../core/method.tpp"
#include "../core/objectproperty.tpp"
#include "../utils/displayname.hpp"

namespace Lua {

ScriptList::ScriptList(Object& _parent, std::string_view parentPropertyName)
  : ObjectList<Script>(_parent, parentPropertyName)
  , status{this, "status", nullptr, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::SubObject | PropertyFlags::NoScript | PropertyFlags::Internal}
  , create{*this, "create",
      [this]()
      {
        auto& world = getWorld(parent());
        return Script::create(world, world.getUniqueId("script"));
      }}
  , delete_{*this, "delete", std::bind(&ScriptList::deleteMethodHandler, this, std::placeholders::_1)}
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
  , clearPersistentVariables{*this, "clear_persistent_variables",
    [this]()
    {
      for(const auto& script : m_items)
      {
        if(Attributes::getEnabled(script->clearPersistentVariables))
        {
          script->clearPersistentVariables();
        }
      }
    }}
{
  status.setValueInternal(std::make_shared<LuaStatus>(*this, status.name()));

  const bool editable = contains(getWorld(parent()).state.value(), WorldState::Edit);

  m_interfaceItems.add(status);

  Attributes::addDisplayName(create, DisplayName::List::create);
  Attributes::addEnabled(create, editable);
  m_interfaceItems.add(create);

  Attributes::addDisplayName(delete_, DisplayName::List::delete_);
  Attributes::addEnabled(delete_, editable);
  m_interfaceItems.add(delete_);

  Attributes::addEnabled(startAll, false);
  m_interfaceItems.add(startAll);

  Attributes::addEnabled(stopAll, false);
  m_interfaceItems.add(stopAll);

  Attributes::addEnabled(clearPersistentVariables, false);
  m_interfaceItems.add(clearPersistentVariables);
}

ScriptList::~ScriptList()
{
  getWorld(parent()).statuses.removeInternal(status.value());
}

TableModelPtr ScriptList::getModel()
{
  return std::make_shared<ScriptListTableModel>(*this);
}

void ScriptList::worldEvent(WorldState state, WorldEvent event)
{
  ObjectList<Script>::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(create, editable);
  Attributes::setEnabled(delete_, editable);
}

void ScriptList::objectAdded(const std::shared_ptr<Script>& /*object*/)
{
  if(m_items.size() == 1)
  {
    getWorld(parent()).statuses.appendInternal(status.value());
  }
  updateEnabled();
}

void ScriptList::objectRemoved(const std::shared_ptr<Script>& /*object*/)
{
  if(empty())
  {
    getWorld(parent()).statuses.removeInternal(status.value());
  }
  updateEnabled();
}

bool ScriptList::isListedProperty(std::string_view name)
{
  return ScriptListTableModel::isListedProperty(name);
}

void ScriptList::updateEnabled()
{
  bool canStart = false;
  bool canStop = false;
  bool canClearPersistentVariables = false;

  for(const auto& script : m_items)
  {
    canStart |= Attributes::getEnabled(script->start);
    canStop |= Attributes::getEnabled(script->stop);
    canClearPersistentVariables |= Attributes::getEnabled(script->clearPersistentVariables);
  }

  Attributes::setEnabled(startAll, canStart);
  Attributes::setEnabled(stopAll, canStop);
  Attributes::setEnabled(clearPersistentVariables, canClearPersistentVariables);
}

}
