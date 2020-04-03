/**
 * Traintastic
 *
 * Copyright (C) 2019 Reinder Feenstra <reinderfeenstra@gmail.com>
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
#include "traintastic.hpp"
#include "world.hpp"

IdObject::IdObject(const std::weak_ptr<World> world, const std::string& _id) :
  Object{},
  m_world{world},
  id{this, "id", _id, PropertyFlags::ReadWrite | PropertyFlags::Store, nullptr,
    [this](std::string& value)
    {
      auto& m = Traintastic::instance->world->m_objects;
      if(m.find(value) != m.end())
        return false;
      auto n = m.extract(id);
      n.key() = value;
      m.insert(std::move(n));
      return true;
    }}
{
  m_interfaceItems.add(id)
    .addAttributeEnabled(false);
}

IdObject::~IdObject()
{
  if(auto world = m_world.lock())
    world->m_objects.erase(id);
}

void IdObject::addToWorld()
{
  if(auto world = m_world.lock())
    world->m_objects.emplace(id, weak_from_this());
}

void IdObject::worldEvent(WorldState state, WorldEvent event)
{
  Object::worldEvent(state, event);

  id.setAttributeEnabled(contains(state, WorldState::Edit));
}
