/**
 * client/src/network/object/trainblockstatus.cpp
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

#include "trainblockstatus.hpp"
#include "../connection.hpp"
#include "../property.hpp"
#include "../objectproperty.hpp"
#include "../error.hpp"

TrainBlockStatus::TrainBlockStatus(const std::shared_ptr<Connection>& connection, Handle handle, const QString& classId_)
  : Object(connection, handle, classId_)
  , m_requestId{Connection::invalidRequestId}
{
}

TrainBlockStatus::~TrainBlockStatus()
{
  if(m_requestId != Connection::invalidRequestId)
    m_connection->cancelRequest(m_requestId);
}

BlockTrainDirection TrainBlockStatus::direction() const
{
  if(m_directionProperty) /*[[likely]]*/
    return m_directionProperty->toEnum<BlockTrainDirection>();
  assert(false);
  return static_cast<BlockTrainDirection>(0);
}

QString TrainBlockStatus::identification() const
{
  if(m_identificationProperty) /*[[likely]]*/
    return m_identificationProperty->toString();
  assert(false);
  return {};
}

void TrainBlockStatus::created()
{
  Object::created();

  m_directionProperty = dynamic_cast<Property*>(getProperty("direction"));
  if(m_directionProperty)
    connect(m_directionProperty, &Property::valueChanged, this,
      [this]()
      {
        emit changed();
      });

  m_identificationProperty = dynamic_cast<Property*>(getProperty("identification"));
  if(m_identificationProperty)
    connect(m_identificationProperty, &Property::valueChanged, this,
      [this]()
      {
        emit changed();
      });

  if((m_trainProperty = dynamic_cast<ObjectProperty*>(getProperty("train")))) /*[[likely]]*/
  {
    connect(m_trainProperty, &ObjectProperty::valueChanged, this, &TrainBlockStatus::updateTrain);
    updateTrain();
  }
}

void TrainBlockStatus::updateTrain()
{
  assert(m_trainProperty);
  if(!m_trainProperty->hasObject() && m_train)
  {
    m_train.reset();
    emit changed();
    return;
  }

  m_requestId = m_trainProperty->getObject(
    [this](const ObjectPtr& object, std::optional<const Error> /*error*/)
    {
      m_requestId = Connection::invalidRequestId;
      m_train = object;
      emit changed();
    }
  );
}
