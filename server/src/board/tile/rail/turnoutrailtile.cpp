/**
 * server/src/board/tile/rail/turnoutrailtile.cpp
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

#include "turnoutrailtile.hpp"
#include "../../../core/attributes.hpp"
#include "../../../world/world.hpp"

TurnoutRailTile::TurnoutRailTile(const std::weak_ptr<World>& world, std::string_view _id, TileId tileId) :
  RailTile(world, _id, tileId),
  name{this, "name", std::string(_id), PropertyFlags::ReadWrite | PropertyFlags::Store},
  position{this, "position", TurnoutPosition::Unknown, PropertyFlags::ReadWrite | PropertyFlags::StoreState},
  nextPosition{*this, "next_position", [this](bool reverse){ doNextPosition(reverse); }}
{
  auto w = world.lock();
  const bool editable = w && contains(w->state.value(), WorldState::Edit);

  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);
  Attributes::addObjectEditor(position, false);
  Attributes::addObjectEditor(nextPosition, false);
  m_interfaceItems.add(nextPosition);
}

void TurnoutRailTile::worldEvent(WorldState state, WorldEvent event)
{
  RailTile::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  name.setAttributeEnabled(editable);
}
