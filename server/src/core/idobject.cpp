/**
 * Traintastic
 *
 * Copyright (C) 2019-2020 Reinder Feenstra <reinderfeenstra@gmail.com>
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
#include "console.hpp"
#include "../world/world.hpp"

IdObject::IdObject(const std::weak_ptr<World>& world, std::string_view _id) :
  Object{},
  m_world{world},
  id{this, "id", std::string(_id.data(), _id.size()), PropertyFlags::ReadWrite | PropertyFlags::Store, nullptr,
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
  auto w = world.lock();
  const bool editable = w && contains(w->state.value(), WorldState::Edit);

  m_interfaceItems.add(id)
    .addAttributeEnabled(editable);
}

IdObject::~IdObject()
{
  assert(m_world.expired());
}
/*
void IdObject::destroy()
{
  if(auto world = m_world.lock())
    world->m_objects.erase(id);
  m_world.reset();
}
*/
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

void IdObject::logDebug(const std::string& message)
{
  Traintastic::instance->console->debug(id, message);
}

void IdObject::logInfo(const std::string& message)
{
  Traintastic::instance->console->info(id, message);
}

void IdObject::logNotice(const std::string& message)
{
  Traintastic::instance->console->notice(id, message);
}

void IdObject::logWarning(const std::string& message)
{
  Traintastic::instance->console->warning(id, message);
}

void IdObject::logError(const std::string& message)
{
  Traintastic::instance->console->error(id, message);
}

void IdObject::logCritical(const std::string& message)
{
  Traintastic::instance->console->critical(id, message);
}

void IdObject::logFatal(const std::string& message)
{
  Traintastic::instance->console->fatal(id, message);
}
