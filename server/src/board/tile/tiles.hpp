/**
 * server/src/board/tile/tiles.hpp
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

#ifndef TRAINTASTIC_SERVER_BOARD_TILE_TILES_HPP
#define TRAINTASTIC_SERVER_BOARD_TILE_TILES_HPP

#include <vector>
#include "tile.hpp"
#include "rail/straightrailtile.hpp"
#include "rail/directioncontrolrailtile.hpp"
#include "rail/onewayrailtile.hpp"
#include "rail/curve45railtile.hpp"
#include "rail/curve90railtile.hpp"
#include "rail/cross45railtile.hpp"
#include "rail/cross90railtile.hpp"
#include "rail/bridge45leftrailtile.hpp"
#include "rail/bridge45rightrailtile.hpp"
#include "rail/bridge90railtile.hpp"
#include "rail/turnout/turnoutleft45railtile.hpp"
#include "rail/turnout/turnoutleft90railtile.hpp"
#include "rail/turnout/turnoutleftcurvedrailtile.hpp"
#include "rail/turnout/turnoutright45railtile.hpp"
#include "rail/turnout/turnoutright90railtile.hpp"
#include "rail/turnout/turnoutrightcurvedrailtile.hpp"
#include "rail/turnout/turnoutwyerailtile.hpp"
#include "rail/turnout/turnout3wayrailtile.hpp"
#include "rail/turnout/turnoutsinglesliprailtile.hpp"
#include "rail/turnout/turnoutdoublesliprailtile.hpp"
#include "rail/signal/signal2aspectrailtile.hpp"
#include "rail/signal/signal3aspectrailtile.hpp"
#include "rail/signal/signalrailtile_ita.hpp"
#include "rail/bufferstoprailtile.hpp"
#include "rail/sensorrailtile.hpp"
#include "rail/blockrailtile.hpp"
#include "rail/tunnelrailtile.hpp"
#include "misc/pushbuttontile.hpp"

struct Tiles
{
  struct Info
  {
    std::string_view classId;
    TileId tileId;
    uint8_t rotates;
    std::vector<std::string_view> menu;
  };

  static constexpr std::string_view classIdPrefix = "board_tile.";

  static const std::vector<Info>& getInfo();

  static std::shared_ptr<Tile> create(World& world, std::string_view classId, std::string_view id = {});

  static bool canUpgradeStraightRail(std::string_view classId);
};

#endif
