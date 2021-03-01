/**
 * server/src/core/object.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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

#include "object.hpp"
#include "abstractmethod.hpp"
#include "abstractproperty.hpp"
#include "traintastic.hpp"

Object::Object() :
  m_dying{false}
{
}

Object::~Object()
{
}

void Object::destroy()
{
  assert(!m_dying);
  if(!m_dying)
  {
    m_dying = true;
    destroying();
  }
}

InterfaceItem* Object::getItem(std::string_view name)
{
  return m_interfaceItems.find(name);
}

AbstractMethod* Object::getMethod(std::string_view name)
{
  return dynamic_cast<AbstractMethod*>(getItem(name));
}

AbstractProperty* Object::getProperty(std::string_view name)
{
  return dynamic_cast<AbstractProperty*>(getItem(name));
}

void Object::worldEvent(WorldState state, WorldEvent event)
{
  for(auto& it : m_interfaceItems)
    if(AbstractProperty* property = dynamic_cast<AbstractProperty*>(&it.second))
      if(contains(property->flags(), PropertyFlags::SubObject))
        property->toObject()->worldEvent(state, event);
}

void Object::logDebug(const std::string& message)
{
  Traintastic::instance->console->debug(getObjectId(), message);
}

void Object::logInfo(const std::string& message)
{
  Traintastic::instance->console->info(getObjectId(), message);
}

void Object::logNotice(const std::string& message)
{
  Traintastic::instance->console->notice(getObjectId(), message);
}

void Object::logWarning(const std::string& message)
{
  Traintastic::instance->console->warning(getObjectId(), message);
}

void Object::logError(const std::string& message)
{
  Traintastic::instance->console->error(getObjectId(), message);
}

void Object::logCritical(const std::string& message)
{
  Traintastic::instance->console->critical(getObjectId(), message);
}

void Object::logFatal(const std::string& message)
{
  Traintastic::instance->console->fatal(getObjectId(), message);
}
