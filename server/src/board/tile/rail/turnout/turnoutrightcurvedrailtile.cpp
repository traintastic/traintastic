/**
 * server/src/board/tile/rail/turnout/turnoutrightcurvedrailtile.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
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

#include "turnoutrightcurvedrailtile.hpp"

TurnoutRightCurvedRailTile::TurnoutRightCurvedRailTile(World& world, std::string_view _id) :
  TurnoutRightRailTile(world, _id, TileId::RailTurnoutRightCurved)
{
}

void TurnoutRightCurvedRailTile::getConnectors(std::vector<Connector>& connectors) const
{
  connectors.emplace_back(location(), rotate, Connector::Type::Rail);
  connectors.emplace_back(location(), rotate + TileRotate::Deg225, Connector::Type::Rail);
  connectors.emplace_back(location(), rotate + TileRotate::Deg270, Connector::Type::Rail);
}
