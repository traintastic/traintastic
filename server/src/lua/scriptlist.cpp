/**
 * server/src/lua/scriptlist.cpp
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

#include "scriptlist.hpp"
#include "scriptlisttablemodel.hpp"
#include "../utils/getworld.hpp"

namespace Lua {

ScriptList::ScriptList(Object& _parent, const std::string& parentPropertyName) :
  ObjectList<Script>(_parent, parentPropertyName),
  add{*this, "add",
    [this]()
    {
      auto world = getWorld(&this->parent());
      if(!world)
        return std::shared_ptr<Script>();
      auto script = Script::create(world, world->getUniqueId("script"));
      addObject(script);
      return script;
    }}
{
  auto world = getWorld(&_parent);
  const bool editable = world && contains(world->state.value(), WorldState::Edit);

  m_interfaceItems.add(add)
    .addAttributeEnabled(editable);
}

TableModelPtr ScriptList::getModel()
{
  return std::make_shared<ScriptListTableModel>(*this);
}

bool ScriptList::isListedProperty(const std::string& name)
{
  return ScriptListTableModel::isListedProperty(name);
}

}
