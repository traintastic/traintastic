/**
 * client/src/network/board.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020 Reinder Feenstra
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

#include "board.hpp"
#include "connection.hpp"
#include <traintastic/network/message.hpp>

Board::Board(const QSharedPointer<Connection>& connection, Handle handle, const QString& classId) :
  Object(connection, handle, classId),
  m_getTileDataRequestId{Connection::invalidRequestId}
{
}

Board::~Board()
{
  if(m_getTileDataRequestId != Connection::invalidRequestId)
    if(auto c = connection())
      c->cancelRequest(m_getTileDataRequestId);
}

void Board::getTileData()
{
  m_getTileDataRequestId = m_connection->getTileData(*this);
}

void Board::getTileDataResponse(const Message& response)
{
  m_getTileDataRequestId = Connection::invalidRequestId;

  while(!response.endOfMessage())
  {
    TileLocation l = response.read<TileLocation>();
    if(static_cast<const TileData*>(response.current())->isLong())
      m_tileData.emplace(l, response.read<TileDataLong>());
    else
      m_tileData.emplace(l, response.read<TileData>());
  }

  emit tileDataChanged();
}
