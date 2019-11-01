/**
 * client/src/network/object.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019 Reinder Feenstra
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
#include "client.hpp"
#include "property.hpp"

Object::Object(Handle handle, const QString& classId) :
  QObject(nullptr),
  m_handle{handle},
  m_classId{classId}
{
}

Object::~Object()
{
  Client::instance->releaseObject(this);
}

const Property* Object::getProperty(const QString& name) const
{
  return dynamic_cast<Property*>(m_interfaceItems.value(name, nullptr));
}

Property* Object::getProperty(const QString& name)
{
  return dynamic_cast<Property*>(m_interfaceItems.value(name, nullptr));
}
