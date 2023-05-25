/**
 * client/src/network/object/blockrailtile.cpp
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

#include "blockrailtile.hpp"
#include "trainblockstatus.hpp"
#include "../connection.hpp"
#include "../objectvectorproperty.hpp"

BlockRailTile::BlockRailTile(const std::shared_ptr<Connection>& connection, Handle handle, const QString& classId_)
  : Object(connection, handle, classId_)
  , m_requestId{Connection::invalidRequestId}
{
}

BlockRailTile::~BlockRailTile()
{
  if(m_requestId != Connection::invalidRequestId)
    m_connection->cancelRequest(m_requestId);
}

void BlockRailTile::created()
{
  Object::created();

  if((m_trainsProperty = dynamic_cast<ObjectVectorProperty*>(getVectorProperty("trains"))))
  {
    connect(m_trainsProperty, &ObjectVectorProperty::valueChanged, this, &BlockRailTile::updateTrains);
    updateTrains();
  }
}

void BlockRailTile::updateTrains()
{
  assert(m_trainsProperty);
  if(m_trainsProperty->empty())
  {
    m_trains.clear();
    emit trainsChanged();
    return;
  }

  m_requestId = m_trainsProperty->getObjects(
    [this](const std::vector<ObjectPtr>& objects, Message::ErrorCode /*ec*/)
    {
      m_requestId = Connection::invalidRequestId;
      m_trains = objects;
      for(auto& status : m_trains)
        connect(static_cast<TrainBlockStatus*>(status.get()), &TrainBlockStatus::changed, this, &BlockRailTile::emitTrainsChanged, Qt::UniqueConnection);
      emit trainsChanged();
    }
  );
}

void BlockRailTile::emitTrainsChanged()
{
  emit trainsChanged();
}
