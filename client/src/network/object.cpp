/**
 * client/src/network/object.cpp
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
#include "connection.hpp"
#include "property.hpp"
#include "method.hpp"

Object::Object(std::shared_ptr<Connection> connection, Handle handle, const QString& classId) :
  QObject(nullptr),
  m_connection{std::move(connection)},
  m_handle{handle},
  m_classId{classId}
{
}

const InterfaceItem* Object::getInterfaceItem(const QString& name) const
{
  return m_interfaceItems.find(name);
}

InterfaceItem* Object::getInterfaceItem(const QString& name)
{
  return m_interfaceItems.find(name);
}

const AbstractProperty* Object::getProperty(const QString& name) const
{
  return dynamic_cast<AbstractProperty*>(m_interfaceItems.find(name));
}

AbstractProperty* Object::getProperty(const QString& name)
{
  return dynamic_cast<AbstractProperty*>(m_interfaceItems.find(name));
}

void Object::setPropertyValue(const QString& name, bool value)
{
  if(AbstractProperty* property = getProperty(name))
    property->setValueBool(value);
  else
    Q_ASSERT(false);
}

const Method* Object::getMethod(const QString& name) const
{
  return dynamic_cast<const Method*>(m_interfaceItems.find(name));
}

Method* Object::getMethod(const QString& name)
{
  return dynamic_cast<Method*>(m_interfaceItems.find(name));
}

void Object::callMethod(const QString& name)
{
  if(Method* method = getMethod(name))
    method->call();
  else
    Q_ASSERT(false);
}
