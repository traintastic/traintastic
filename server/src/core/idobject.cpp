/**
 * server/src/core/idobject.cpp
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

#include "idobject.hpp"
#include "../traintastic/traintastic.hpp"
#include "../world/getworld.hpp"
#include "attributes.hpp"
#include "isvalidobjectid.hpp"
#include "../utils/displayname.hpp"

IdObject::IdObject(World& world, std::string_view _id) :
  m_world{world},
  id{this, "id", std::string(_id.data(), _id.size()), PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly,
    nullptr,
    [this](std::string& value)
    {
      if(!isValidObjectId(value))
        throw invalid_value_error();
      auto& m = m_world.m_objects;
      if(m.find(value) != m.end())
        return false;
      auto n = m.extract(id);
      n.key() = value;
      m.insert(std::move(n));
      return true;
    }}
{
  const bool editable = contains(m_world.state.value(), WorldState::Edit);

  Attributes::addDisplayName(id, DisplayName::Object::id);
  Attributes::addEnabled(id, editable);
  m_interfaceItems.add(id);
}

IdObject::~IdObject()
{
  //assert(m_world.expired()); // is destroy() called ??
}

void IdObject::destroying()
{
  m_world.m_objects.erase(id);
  Object::destroying();
}

void IdObject::addToWorld()
{
  m_world.m_objects.emplace(id, weak_from_this());
}

void IdObject::worldEvent(WorldState state, WorldEvent event)
{
  Object::worldEvent(state, event);

  Attributes::setEnabled(id, contains(state, WorldState::Edit));
}
