/**
 * client/src/network/board.hpp
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

#ifndef TRAINTASTIC_CLIENT_NETWORK_BOARD_HPP
#define TRAINTASTIC_CLIENT_NETWORK_BOARD_HPP

#include "object.hpp"
#include <QString>
#include <unordered_map>
#include <optional>
#include <traintastic/enum/tristate.hpp>
#include <traintastic/board/tilelocation.hpp>
#include <traintastic/board/tiledata.hpp>
#include "objectptr.hpp"

struct Error;

class Board final : public Object
{
  Q_OBJECT

  friend class Connection;

  public:
    using TileDataMap = std::unordered_map<TileLocation, TileData, TileLocationHash>;
    using TileObjectMap = std::unordered_map<TileLocation, ObjectPtr, TileLocationHash>;

    struct TileInfo
    {
      QString classId;
      TileId tileId;
      uint8_t rotates;
      QStringList menu;
    };

    static std::vector<TileInfo> tileInfo;

  protected:
    TileDataMap m_tileData;
    TileObjectMap m_tileObjects;
    int m_getTileDataRequestId;

    void getTileDataResponse(const Message& response);
    void processMessage(const Message& message) final;

  public:
    inline static const QString classId = QStringLiteral("board");

    Board(std::shared_ptr<Connection> connection, Handle handle);
    ~Board() final;

    void getTileData();
    const TileDataMap& tileData() const { return m_tileData; }

    const TileObjectMap& tileObjects() const { return m_tileObjects; }

    bool getTileOrigin(TileLocation& l) const;
    TileId getTileId(TileLocation l) const;
    ObjectPtr getTileObject(TileLocation l) const;

    int addTile(int16_t x, int16_t y, TileRotate rotate, const QString& id, bool replace, std::function<void(const bool&, std::optional<const Error>)> callback);
    int moveTile(int16_t xFrom, int16_t yFrom, int16_t xTo, int16_t yTo, TileRotate rotate, bool replace, std::function<void(const bool&, std::optional<const Error>)> callback);
    int resizeTile(int16_t x, int16_t y, uint8_t w, uint8_t h, std::function<void(const bool&, std::optional<const Error>)> callback);
    int deleteTile(int16_t x, int16_t y, std::function<void(const bool&, std::optional<const Error>)> callback);

  signals:
    void tileDataChanged();
    void tileObjectAdded(int16_t x, int16_t y, const ObjectPtr& object);
};

#endif
