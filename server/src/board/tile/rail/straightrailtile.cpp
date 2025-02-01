/**
 * server/src/board/tile/rail/straightrailtile.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020,2022,2024 Reinder Feenstra
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

#include "straightrailtile.hpp"

StraightRailTile::StraightRailTile(World& world, std::string_view _id, TileId tileId_) :
  RailTile(world, _id, tileId_)
{
}

void StraightRailTile::getConnectors(std::vector<Connector>& connectors) const
{
  connectors.emplace_back(location(), rotate, Connector::Type::Rail);
  connectors.emplace_back(location(), rotate + TileRotate::Deg180, Connector::Type::Rail);
}
