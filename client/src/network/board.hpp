/**
 * client/src/network/board.hpp
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

#ifndef TRAINTASTIC_CLIENT_NETWORK_BOARD_HPP
#define TRAINTASTIC_CLIENT_NETWORK_BOARD_HPP

#include "object.hpp"
#include <QString>
#include <unordered_map>
#include <traintastic/enum/tristate.hpp>
#include <traintastic/board/tilelocation.hpp>
#include <traintastic/board/tiledata.hpp>
#include <traintastic/network/message.hpp>

class Connection;
class Message;

class Board final : public Object
{
  Q_OBJECT

  friend class Connection;

  public:
    using TileDataMap = std::unordered_map<TileLocation, TileDataLong, TileLocationHash>;

  protected:
    TileDataMap m_tileData;
    int m_getTileDataRequestId;

    void getTileDataResponse(const Message& response);
    void processMessage(const Message& message);

  public:
    inline static const QString classId = QStringLiteral("board");

    Board(const QSharedPointer<Connection>& connection, Handle handle, const QString& classId);
    ~Board() final;

    void getTileData();
    const TileDataMap& tileData() const { return m_tileData; }

    int addTile(int16_t x, int16_t y, TileRotate rotate, const QString& id, bool replace, std::function<void(const bool&, Message::ErrorCode)> callback);
    int deleteTile(int16_t x, int16_t y, std::function<void(const bool&, Message::ErrorCode)> callback);

  signals:
    void tileDataChanged();
};

#endif
