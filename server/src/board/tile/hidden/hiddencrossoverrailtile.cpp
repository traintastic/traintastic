/**
 * server/src/board/tile/hidden/hiddencrossoverrailtile.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#include "hiddencrossoverrailtile.hpp"
#include <traintastic/enum/crossstate.hpp>

HiddenCrossOverRailTile::HiddenCrossOverRailTile(World& world)
  : HiddenTile(world, TileId::HiddenRailCrossOver)
  , m_node{*this, 4}
  , m_crossState{CrossState::Unset}
{
}

std::string_view HiddenCrossOverRailTile::getClassId() const
{
  assert(false);
  return {};
}

void HiddenCrossOverRailTile::getConnectors(std::vector<Connector>& connectors) const
{
  //      x  x+1
  //     +--+--+
  // y   |  |  |
  //     +--X--+
  // y+1 |  |  |
  //     +--+--+
  //
  // The hidden crossing is actually at (x+0.5, y+0.5), but that can't be stored.
  // So we store (x, y) and trick it a bit.
  connectors.emplace_back(location().adjusted(0, 1), Connector::Direction::NorthEast, Connector::Type::Rail);
  connectors.emplace_back(location(), Connector::Direction::SouthEast, Connector::Type::Rail);
  connectors.emplace_back(location().adjusted(1, 0), Connector::Direction::SouthWest, Connector::Type::Rail);
  connectors.emplace_back(location().adjusted(1, 1), Connector::Direction::NorthWest, Connector::Type::Rail);
}

bool HiddenCrossOverRailTile::reserve(CrossState crossState, bool dryRun)
{
  if(m_crossState != CrossState::Unset)
  {
    return false;
  }

  if(!dryRun)
  {
    m_crossState = crossState;
  }

  return true;
}

bool HiddenCrossOverRailTile::release(bool dryRun)
{
  if(!dryRun)
  {
    m_crossState = CrossState::Unset;
  }
  return true;
}
