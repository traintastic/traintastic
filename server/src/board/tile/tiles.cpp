/**
 * server/src/board/tile/tiles.cpp
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

#include "tiles.hpp"
#include "../../utils/ifclassidcreate.hpp"
#include "../../world/world.hpp"

std::shared_ptr<Tile> Tiles::create(const std::shared_ptr<World>& world, std::string_view classId, std::string_view id)
{
  IF_CLASSID_CREATE(StraightRailTile)
  IF_CLASSID_CREATE(Curve45RailTile)
  IF_CLASSID_CREATE(Curve90RailTile)
  IF_CLASSID_CREATE(Bridge45LeftRailTile)
  IF_CLASSID_CREATE(Bridge45RightRailTile)
  IF_CLASSID_CREATE(Bridge90RailTile)
  IF_CLASSID_CREATE(Cross45RailTile)
  IF_CLASSID_CREATE(Cross90RailTile)
  IF_CLASSID_CREATE(TurnoutLeft45RailTile)
  IF_CLASSID_CREATE(TurnoutLeft90RailTile)
  IF_CLASSID_CREATE(TurnoutLeftCurvedRailTile)
  IF_CLASSID_CREATE(TurnoutRight45RailTile)
  IF_CLASSID_CREATE(TurnoutRight90RailTile)
  IF_CLASSID_CREATE(TurnoutRightCurvedRailTile)
  IF_CLASSID_CREATE(TurnoutWyeRailTile)
  IF_CLASSID_CREATE(Turnout3WayRailTile)
  IF_CLASSID_CREATE(TurnoutSingleSlipRailTile)
  IF_CLASSID_CREATE(TurnoutDoubleSlipRailTile)
  IF_CLASSID_CREATE(Signal2AspectRailTile)
  IF_CLASSID_CREATE(Signal3AspectRailTile)
  IF_CLASSID_CREATE(BufferStopRailTile)
  IF_CLASSID_CREATE(SensorRailTile)
  IF_CLASSID_CREATE(BlockRailTile)
  IF_CLASSID_CREATE(TunnelRailTile)
  return std::shared_ptr<Tile>();
}

bool Tiles::canUpgradeStraightRail(std::string_view classId)
{
  return
    (classId == TunnelRailTile::classId) ||
    (classId == BlockRailTile::classId) ||
    (classId == SensorRailTile::classId) ||
    (classId == Signal2AspectRailTile::classId) ||
    (classId == Signal3AspectRailTile::classId);
}
