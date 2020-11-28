/**
 * server/src/board/tile/tiles.hpp
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

#ifndef TRAINTASTIC_SERVER_BOARD_TILE_TILES_HPP
#define TRAINTASTIC_SERVER_BOARD_TILE_TILES_HPP

#include "tile.hpp"
#include "../../utils/makearray.hpp"
#include "rail/straightrailtile.hpp"
#include "rail/curve45railtile.hpp"
#include "rail/curve90railtile.hpp"
#include "rail/cross45railtile.hpp"
#include "rail/cross90railtile.hpp"
#include "rail/turnoutleftrailtile.hpp"
#include "rail/turnoutrightrailtile.hpp"
#include "rail/turnoutwyerailtile.hpp"
#include "rail/turnout3wayrailtile.hpp"
#include "rail/turnoutsinglesliprailtile.hpp"
#include "rail/turnoutdoublesliprailtile.hpp"
#include "rail/signal2aspectrailtile.hpp"
#include "rail/signal3aspectrailtile.hpp"
#include "rail/bufferstoprailtile.hpp"
#include "rail/sensorrailtile.hpp"
#include "rail/blockrailtile.hpp"

struct Tiles
{
  static constexpr std::string_view classIdPrefix = "board_tile.";

  static constexpr auto classList = makeArray(
    StraightRailTile::classId,
    Curve45RailTile::classId,
    Curve90RailTile::classId,
    Cross45RailTile::classId,
    Cross90RailTile::classId,
    TurnoutLeftRailTile::classId,
    TurnoutRightRailTile::classId,
    TurnoutWyeRailTile::classId,
    Turnout3WayRailTile::classId,
    TurnoutSingleSlipRailTile::classId,
    TurnoutDoubleSlipRailTile::classId,
    Signal2AspectRailTile::classId,
    Signal3AspectRailTile::classId,
    BufferStopRailTile::classId,
    SensorRailTile::classId,
    BlockRailTile::classId
  );

  static std::shared_ptr<Tile> create(const std::weak_ptr<World>& world, std::string_view classId, std::string_view id = {});
};

#endif
