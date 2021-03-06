/**
 * server/src/board/tile/rail/blockrailtile.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2021 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_BOARD_TILE_RAIL_BLOCKRAILTILE_HPP
#define TRAINTASTIC_SERVER_BOARD_TILE_RAIL_BLOCKRAILTILE_HPP

#include "railtile.hpp"
#include "../../../core/objectproperty.hpp"
#include "../../../core/vectorproperty.hpp"
#include "../../../enum/blockstate.hpp"
#include "../../../hardware/input/map/blockinputmap.hpp"

class BlockInputMapItem;

class BlockRailTile : public RailTile
{
  CLASS_ID("board_tile.rail.block")
  DEFAULT_ID("block")
  CREATE(BlockRailTile)

  protected:
    void worldEvent(WorldState state, WorldEvent event) final;
    void setRotate(TileRotate value) final;

    void updateState();

  public:
    Property<std::string> name;
    ObjectProperty<BlockInputMap> inputMap;
    Property<BlockState> state;
    VectorProperty<SensorState> sensorStates;

    BlockRailTile(const std::weak_ptr<World>& world, std::string_view _id);

    void inputItemValueChanged(BlockInputMapItem& item);
};

#endif
