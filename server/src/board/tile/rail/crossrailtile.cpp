/**
 * server/src/board/tile/rail/crossrailtile.cpp
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

#include "crossrailtile.hpp"
#include <traintastic/enum/crossstate.hpp>

CrossRailTile::CrossRailTile(World& world, std::string_view _id, TileId tileId_)
  : RailTile(world, _id, tileId_)
  , m_node{*this, 4}
  , m_crossState{CrossState::Unset}
{
  assert(isRailCross(tileId_));
}

bool CrossRailTile::reserve(CrossState crossState, bool dryRun)
{
  if(m_crossState != CrossState::Unset)
  {
    return false;
  }

  if(!dryRun)
  {
    m_crossState = crossState;
    RailTile::setReservedState(static_cast<uint8_t>(m_crossState));
  }

  return true;
}

bool CrossRailTile::release(bool dryRun)
{
  //! \todo check occupancy sensor, once supported

  if(!dryRun)
  {
    m_crossState = CrossState::Unset;
    RailTile::release();
  }
  return true;
}
