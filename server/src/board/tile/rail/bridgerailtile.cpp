/**
 * server/src/board/tile/rail/bridgerailtile.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2023 Reinder Feenstra
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

#include "bridgerailtile.hpp"

BridgeRailTile::BridgeRailTile(World& world, std::string_view _id, TileId tileId_)
  : RailTile(world, _id, tileId_)
  , m_node{*this, 4}
{
  assert(isRailBridge(tileId_));
}

bool BridgeRailTile::reserve(BridgePath path, bool dryRun)
{
  const uint8_t mask = 1 << static_cast<uint8_t>(path);

  if((reservedState() & mask))
  {
    return false; // already reserved
  }

  if(!dryRun)
  {
    RailTile::reserve(reservedState() | mask);
  }

  return true;
}
