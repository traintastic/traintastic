/**
 * server/src/core/stateobject.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#include "stateobject.hpp"
#include "../world/world.hpp"

void StateObject::addToWorld(World& world, StateObject& object)
{
  world.m_objects.emplace(object.getObjectId(), object.weak_from_this());
}

void StateObject::removeFromWorld(World& world, StateObject& object)
{
  world.m_objects.erase(object.m_id);
}

StateObject::StateObject(std::string id)
  : m_id{std::move(id)}
{
  assert(!m_id.empty());
}

void StateObject::save(WorldSaver& saver, nlohmann::json& data, nlohmann::json& state) const
{
#ifndef NDEBUG
  for(const auto& it : m_interfaceItems)
    if(const auto* p = dynamic_cast<const BaseProperty*>(&it.second))
      assert(!p->isStoreable()); // A StateObject may no have storable properties
#endif
  Object::save(saver, data, state);
  data["id"] = m_id;
}
