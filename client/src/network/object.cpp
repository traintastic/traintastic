/**
 * client/src/network/object.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2024 Reinder Feenstra
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
#include "objectproperty.hpp"
#include "abstractvectorproperty.hpp"
#include "objectvectorproperty.hpp"
#include "unitproperty.hpp"
#include "method.hpp"
#include "event.hpp"

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

bool Object::getPropertyValueBool(const QString& name, bool defaultValue) const
{
  if(const auto* property = getProperty(name); property && property->type() == ValueType::Boolean)
    return property->toBool();
  return defaultValue;
}

int Object::getPropertyValueInt(const QString& name, int defaultValue) const
{
  if(const auto* property = getProperty(name); property && property->type() == ValueType::Integer)
    return property->toInt();
  return defaultValue;
}

QString Object::getPropertyValueString(const QString& name, const QString& defaultValue) const
{
  if(const auto* property = getProperty(name); property && property->type() == ValueType::String)
    return property->toString();
  return defaultValue;
}

const UnitProperty* Object::getUnitProperty(const QString& name) const
{
  return dynamic_cast<UnitProperty*>(m_interfaceItems.find(name));
}

UnitProperty* Object::getUnitProperty(const QString& name)
{
  return dynamic_cast<UnitProperty*>(m_interfaceItems.find(name));
}

ObjectProperty* Object::getObjectProperty(const QString& name) const
{
  return dynamic_cast<ObjectProperty*>(m_interfaceItems.find(name));
}

ObjectProperty* Object::getObjectProperty(const QString& name)
{
  return dynamic_cast<ObjectProperty*>(m_interfaceItems.find(name));
}

const AbstractVectorProperty* Object::getVectorProperty(const QString& name) const
{
  return dynamic_cast<AbstractVectorProperty*>(m_interfaceItems.find(name));
}

AbstractVectorProperty* Object::getVectorProperty(const QString& name)
{
  return dynamic_cast<AbstractVectorProperty*>(m_interfaceItems.find(name));
}

ObjectVectorProperty* Object::getObjectVectorProperty(const QString& name)
{
  return dynamic_cast<ObjectVectorProperty*>(m_interfaceItems.find(name));
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

const Event* Object::getEvent(const QString& name) const
{
  return dynamic_cast<const Event*>(m_interfaceItems.find(name));
}

Event* Object::getEvent(const QString& name)
{
  return dynamic_cast<Event*>(m_interfaceItems.find(name));
}

void Object::callMethod(const QString& name)
{
  if(Method* method = getMethod(name))
    method->call();
  else
    Q_ASSERT(false);
}

void Object::processMessage(const Message& message)
{
  switch(message.command())
  {
    case Message::Command::ObjectEventFired:
    {
      if(Event* event = getEvent(QString::fromLatin1(message.read<QByteArray>())))
      {
        const auto& argumentTypes = event->argumentTypes();
        const auto argumentCount = message.read<uint32_t>();
        assert(argumentTypes.size() == argumentCount);
        QVariantList arguments;
        arguments.reserve(argumentCount);
        for(uint32_t i = 0; i < argumentCount; i++)
        {
          switch(argumentTypes[i])
          {
            case ValueType::Boolean:
              arguments.push_back(message.read<bool>());
              break;

            case ValueType::Integer:
            case ValueType::Enum:
            case ValueType::Set:
              arguments.push_back(message.read<qint64>());
              break;

            case ValueType::Float:
              arguments.push_back(message.read<double>());
              break;

            case ValueType::String:
            case ValueType::Object:
              arguments.push_back(QString::fromUtf8(message.read<QByteArray>()));
              break;

            default:
              assert(false);
              return;
          }
        }
        emit event->fired(arguments);
      }
      break;
    }
    default:
      assert(false);
      break;
  }
}
