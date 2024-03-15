/**
 * server/src/board/tile/tile.hpp
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

#ifndef TRAINTASTIC_SERVER_BOARD_TILE_TILE_HPP
#define TRAINTASTIC_SERVER_BOARD_TILE_TILE_HPP

#include <optional>
#include "../../core/idobject.hpp"
#include "../map/connector.hpp"
#include <traintastic/board/tiledata.hpp>
#include <traintastic/board/tilelocation.hpp>
#include "../../enum/tilerotate.hpp"

class Node;
class Board;

class Tile : public IdObject
{
  friend class Board;
  friend class WorldLoader;

  protected:
    const TileId m_tileId;

    Tile(World& world, std::string_view _id, TileId tileId);

    Board& getBoard();

    virtual uint8_t reservedState() const
    {
      return 0;
    }

    virtual void boardModified() {}
    virtual void setRotate(TileRotate value) { rotate.setValueInternal(value); }
    bool resize(uint8_t w, uint8_t h);

  public:
    static constexpr std::string_view defaultId = "tile";

    Property<int16_t> x;
    Property<int16_t> y;
    Property<TileRotate> rotate;
    Property<uint8_t> height;
    Property<uint8_t> width;

    TileId tileId() const { return m_tileId; }
    inline TileLocation location() const { return {x.value(), y.value()}; }
    inline TileData data() const { return TileData{m_tileId, rotate, width, height, reservedState()}; }

    virtual std::optional<std::reference_wrapper<const Node>> node() const { return {}; }
    virtual std::optional<std::reference_wrapper<Node>> node() { return {}; }
    virtual void getConnectors(std::vector<Connector>& /*connectors*/) const {}
};

#endif
