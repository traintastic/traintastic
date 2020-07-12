/**
 * server/src/core/object.cpp
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

#include "object.hpp"
#include "abstractmethod.hpp"
#include "abstractproperty.hpp"

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

InterfaceItem* Object::getItem(const std::string& name)
{
  return m_interfaceItems.find(name);
}

AbstractMethod* Object::getMethod(const std::string& name)
{
  return dynamic_cast<AbstractMethod*>(getItem(name));
}

AbstractProperty* Object::getProperty(const std::string& name)
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
