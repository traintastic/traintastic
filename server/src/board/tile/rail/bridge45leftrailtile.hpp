/**
 * server/src/board/tile/rail/bridge45leftrailtile.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_BOARD_TILE_RAIL_BRIDGE45LEFTRAILTILE_HPP
#define TRAINTASTIC_SERVER_BOARD_TILE_RAIL_BRIDGE45LEFTRAILTILE_HPP

#include "railtile.hpp"

class Bridge45LeftRailTile : public RailTile
{
  CLASS_ID("board_tile.rail.bridge_45_left")
  CREATE(Bridge45LeftRailTile)

  public:
    Bridge45LeftRailTile(const std::weak_ptr<World>& world, std::string_view _id);
};

#endif
