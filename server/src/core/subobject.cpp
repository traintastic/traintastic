/**
 * server/src/core/subobject.cpp
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

#include "subobject.hpp"
#include "idobject.hpp"
#include "../world/world.hpp"
#include "traintastic.hpp"

SubObject::SubObject(Object& _parent, const std::string& parentPropertyName) :
  Object(),
  m_parent{_parent},
  m_parentPropertyName{parentPropertyName}
{
}

std::string SubObject::id() const
{
  std::string value;
  if(IdObject* object = dynamic_cast<IdObject*>(&m_parent))
    value = object->id;
  else if(SubObject* object = dynamic_cast<SubObject*>(&m_parent))
    value = object->id();
  else if(dynamic_cast<World*>(&m_parent))
    value = World::classId;
  else
    assert(false);
  value += ".";
  value += m_parentPropertyName;
  return value;
}

void SubObject::logDebug(const std::string& message)
{
  Traintastic::instance->console->debug(id(), message);
}

void SubObject::logInfo(const std::string& message)
{
  Traintastic::instance->console->info(id(), message);
}

void SubObject::logNotice(const std::string& message)
{
  Traintastic::instance->console->notice(id(), message);
}

void SubObject::logWarning(const std::string& message)
{
  Traintastic::instance->console->warning(id(), message);
}

void SubObject::logError(const std::string& message)
{
  Traintastic::instance->console->error(id(), message);
}

void SubObject::logCritical(const std::string& message)
{
  Traintastic::instance->console->critical(id(), message);
}

void SubObject::logFatal(const std::string& message)
{
  Traintastic::instance->console->fatal(id(), message);
}
