/**
 * server/src/board/board.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_BOARD_BOARD_HPP
#define TRAINTASTIC_SERVER_BOARD_BOARD_HPP

#include "../core/idobject.hpp"
#include <unordered_map>
#include "../core/method.hpp"
#include <traintastic/board/tilelocation.hpp>
#include <traintastic/enum/tilerotate.hpp>

class Tile;
struct TileData;

class Board : public IdObject
{
  public:
    using TileMap = std::unordered_map<TileLocation, std::shared_ptr<Tile>, TileLocationHash>;

  private:
    bool m_modified = false;

    void modified();
    void removeTile(int16_t x, int16_t y);
    void updateSize(bool allowShrink = false);

  protected:
    TileMap m_tiles;

    void addToWorld() final;
    void destroying() final;
    void load(WorldLoader& loader, const nlohmann::json& data) final;
    void save(WorldSaver& saver, nlohmann::json& data, nlohmann::json& state) const final;
    void worldEvent(WorldState state, WorldEvent event) override;
    void loaded() final;

  public:
    static constexpr int16_t sizeMax = 1000;
    static constexpr int16_t sizeMin = -sizeMax;

    CLASS_ID("board")
    CREATE_DEF(Board)

    Property<std::string> name;
    Property<int16_t> left;
    Property<int16_t> top;
    Property<int16_t> right;
    Property<int16_t> bottom;
    Method<bool(int16_t, int16_t, TileRotate, std::string_view, bool)> addTile;
    Method<bool(int16_t, int16_t, int16_t, int16_t, TileRotate, bool)> moveTile;
    Method<bool(int16_t, int16_t, uint8_t, uint8_t)> resizeTile;
    Method<bool(int16_t, int16_t)> deleteTile;
    Method<void()> resizeToContents;

    boost::signals2::signal<void (Board&, const TileLocation&, const TileData&)> tileDataChanged;

    Board(World& world, std::string_view _id);

    const TileMap& tileMap() const { return m_tiles; }

    bool isTile(TileLocation l)
    {
      auto it = m_tiles.find(l);
      return it != m_tiles.end();
    }

    std::shared_ptr<const Tile> getTile(TileLocation l) const
    {
      if(auto it = m_tiles.find(l); it != m_tiles.end())
        return it->second;

      return {};
    }

    std::shared_ptr<Tile> getTile(TileLocation l)
    {
      if(auto it = m_tiles.find(l); it != m_tiles.end())
        return it->second;

      return {};
    }
};

#endif
