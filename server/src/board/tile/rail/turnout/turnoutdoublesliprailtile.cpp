/**
 * server/src/board/tile/rail/turnout/turnoutdoublesliprailtile.cpp
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

#include "turnoutdoublesliprailtile.hpp"
#include "../../../../core/attributes.hpp"
#include "../../../../utils/makearray.hpp"

TurnoutDoubleSlipRailTile::TurnoutDoubleSlipRailTile(const std::weak_ptr<World>& world, std::string_view _id) :
  TurnoutRailTile(world, _id, TileId::RailTurnoutDoubleSlip)
{
  outputMap.setValueInternal(std::make_shared<TurnoutOutputMap>(*this, outputMap.name(), std::initializer_list<TurnoutPosition>{TurnoutPosition::Crossed, TurnoutPosition::Diverged}));

  Attributes::addValues(position, makeArray(TurnoutPosition::Crossed, TurnoutPosition::Diverged, TurnoutPosition::Unknown));
  m_interfaceItems.add(position);
}

void TurnoutDoubleSlipRailTile::doNextPosition(bool)
{
  position = (position == TurnoutPosition::Crossed) ? TurnoutPosition::Diverged : TurnoutPosition::Crossed;
}
