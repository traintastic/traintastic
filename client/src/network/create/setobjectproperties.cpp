/**
 * client/src/network/create/setobjectproperties.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#include "setobjectproperties.hpp"
#include "../connection.hpp"
#include "../error.hpp"
#include "../object.hpp"
#include "../abstractproperty.hpp"
#include "../objectproperty.hpp"

SetObjectProperties::SetObjectProperties(ObjectPtr object, Properties properties)
  : m_object{std::move(object)}
  , m_properties{std::move(properties)}
  , m_requestId{Connection::invalidRequestId}
{
  setProperties();
}

SetObjectProperties::SetObjectProperties(ObjectProperty& objectProperty, Properties properties)
  : m_objectProperty{&objectProperty}
  , m_properties{std::move(properties)}
{
  m_requestId = m_objectProperty->getObject(
    [this](const ObjectPtr& object, std::optional<const Error> /*error*/)
    {
      m_requestId = Connection::invalidRequestId;
      if(!m_canceled)
      {
        m_object = object;
        setProperties();
      }
    });
}

SetObjectProperties::~SetObjectProperties()
{
  if(!m_promise.isFinished() && !m_canceled)
  {
    cancel();
  }
  if(m_requestId != Connection::invalidRequestId)
  {
    m_objectProperty->object().connection()->cancelRequest(m_requestId);
  }
}

void SetObjectProperties::cancel()
{
  m_canceled = true;
  if(!m_promise.isFinished())
  {
    m_promise.reportCanceled();
  }
}

void SetObjectProperties::setProperties()
{
  assert(m_object);

  std::map<ObjectProperty*, Properties> subObjects;

  for(const auto& it : m_properties)
  {
    if(int pos = it.first.indexOf(QChar('.')); pos >= 0) // property of sub object
    {
      qDebug() << it.first << it.first.left(pos);
      auto* objectProperty = m_object->getObjectProperty(it.first.left(pos));
      if(objectProperty)
      {
        subObjects[objectProperty].emplace_back(it.first.mid(pos + 1), it.second);
      }
    }
    else if(auto* property = m_object->getProperty(it.first))
    {
      qDebug() << it.first << it.second;
      property->setValueVariant(it.second);
    }
  }

  if(subObjects.empty()) // we're done
  {
    m_promise.reportFinished(&m_object);
  }
  else
  {
    for(const auto& it : subObjects)
    {
      m_setObjectProperties.emplace_back(std::make_shared<SetObjectProperties>(*it.first, it.second));
      m_setObjectPropertiesFutureWatcher.emplace_back(std::make_unique<QFutureWatcher<ObjectPtr>>());
      m_setObjectPropertiesFutureWatcher.back()->setFuture(m_setObjectProperties.back()->future());
      QObject::connect(m_setObjectPropertiesFutureWatcher.back().get(), &QFutureWatcher<ObjectPtr>::finished,
        [this]()
        {
          if(!m_canceled && ++m_setObjectPropertiesFinishedCount == m_setObjectProperties.size())
          {
            m_promise.reportFinished(&m_object);
          }
        });
    }
  }
}
