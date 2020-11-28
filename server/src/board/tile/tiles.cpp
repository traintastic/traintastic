/**
 * server/src/board/tile/tiles.cpp
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

#include "tiles.hpp"
#include "../../utils/ifclassidcreate.hpp"

std::shared_ptr<Tile> Tiles::create(const std::weak_ptr<World>& world, std::string_view classId, std::string_view id)
{
  IF_CLASSID_CREATE(StraightRailTile)
  IF_CLASSID_CREATE(Curve45RailTile)
  IF_CLASSID_CREATE(Curve90RailTile)
  IF_CLASSID_CREATE(Cross45RailTile)
  IF_CLASSID_CREATE(Cross90RailTile)
  IF_CLASSID_CREATE(TurnoutLeftRailTile)
  IF_CLASSID_CREATE(TurnoutRightRailTile)
  IF_CLASSID_CREATE(TurnoutWyeRailTile)
  IF_CLASSID_CREATE(Turnout3WayRailTile)
  IF_CLASSID_CREATE(TurnoutSingleSlipRailTile)
  IF_CLASSID_CREATE(TurnoutDoubleSlipRailTile)
  IF_CLASSID_CREATE(Signal2AspectRailTile)
  IF_CLASSID_CREATE(Signal3AspectRailTile)
  IF_CLASSID_CREATE(BufferStopRailTile)
  return std::shared_ptr<Tile>();
}
