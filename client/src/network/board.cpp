/**
 * client/src/network/board.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2023 Reinder Feenstra
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
#include "callmethod.hpp"

std::vector<Board::TileInfo> Board::tileInfo;

Board::Board(std::shared_ptr<Connection> connection, Handle handle) :
  Object(std::move(connection), handle, classId),
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

bool Board::getTileOrigin(TileLocation& l) const
{
  if(auto it = m_tileData.find(l); it != m_tileData.end())
    return true;

  for(const auto& it : m_tileData)
    if(l.x >= it.first.x && l.x < it.first.x + it.second.width() &&
        l.y >= it.first.y && l.y < it.first.y + it.second.height())
    {
      l = it.first;
      return true;
    }

  return false;
}

TileId Board::getTileId(TileLocation l) const
{
  if(auto it = m_tileData.find(l); it != m_tileData.end())
    return it->second.id();

  for(const auto& it : m_tileData)
    if(l.x >= it.first.x && l.x < it.first.x + it.second.width() &&
        l.y >= it.first.y && l.y < it.first.y + it.second.height())
    {
      return it.second.id();
    }

  return TileId::None;
}

ObjectPtr Board::getTileObject(TileLocation l) const
{
  auto itObject = m_tileObjects.find(l);

  if(itObject == m_tileObjects.end())
    for(const auto& itData : m_tileData)
      if(l.x >= itData.first.x && l.x < itData.first.x + itData.second.width() &&
          l.y >= itData.first.y && l.y < itData.first.y + itData.second.height())
        itObject = m_tileObjects.find(itData.first);

  if(itObject != m_tileObjects.end())
    return itObject->second;
  else
    return ObjectPtr();
}

int Board::addTile(int16_t x, int16_t y, TileRotate rotate, const QString& id, bool replace, std::function<void(const bool&, std::optional<const Error>)> callback)
{
  return ::callMethod(*m_connection, *getMethod("add_tile"), std::move(callback), x, y, rotate, id, replace);
}

int Board::moveTile(int16_t xFrom, int16_t yFrom, int16_t xTo, int16_t yTo, TileRotate rotate, bool replace, std::function<void(const bool&, std::optional<const Error>)> callback)
{
  return ::callMethod(*m_connection, *getMethod("move_tile"), std::move(callback), xFrom, yFrom, xTo, yTo, rotate, replace);
}

int Board::resizeTile(int16_t x, int16_t y, uint8_t w, uint8_t h, std::function<void(const bool&, std::optional<const Error>)> callback)
{
  return ::callMethod(*m_connection, *getMethod("resize_tile"), std::move(callback), x, y, w, h);
}

int Board::deleteTile(int16_t x, int16_t y, std::function<void(const bool&, std::optional<const Error>)> callback)
{
  return ::callMethod(*m_connection, *getMethod("delete_tile"), std::move(callback), x, y);
}

void Board::getTileDataResponse(const Message& response)
{
  m_getTileDataRequestId = Connection::invalidRequestId;

  while(!response.endOfMessage())
  {
    TileLocation l = response.read<TileLocation>();
    TileData data = response.read<TileData>();
    m_tileData.emplace(l, data);
    if(data.isActive())
      emit tileObjectAdded(l.x, l.y, m_tileObjects.emplace(l, m_connection->readObject(response)).first->second);
  }

  emit tileDataChanged();
}

void Board::processMessage(const Message& message)
{
  switch(message.command())
  {
    case Message::Command::BoardTileDataChanged:
    {
      TileLocation l = message.read<TileLocation>();
      TileData data = message.read<TileData>();
      if(!data) // no tile
      {
        auto it = m_tileData.find(l);
        if(it != m_tileData.end())
          m_tileData.erase(it);
      }
      else
        m_tileData[l] = data;

      if(data.isPassive())
      {
        auto it = m_tileObjects.find(l);
        if(it != m_tileObjects.end())
          m_tileObjects.erase(it);
      }
      else
      {
        m_tileObjects[l] = m_connection->readObject(message);
        emit tileObjectAdded(l.x, l.y, m_tileObjects[l]);
      }

      emit tileDataChanged();
      break;
    }
    default:
      Object::processMessage(message);
      break;
  }
}
