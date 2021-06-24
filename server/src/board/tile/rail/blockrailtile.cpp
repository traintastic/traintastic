/**
 * server/src/board/tile/rail/blockrailtile.cpp
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

#include "blockrailtile.hpp"
#include "../../../world/world.hpp"
#include "../../../core/attributes.hpp"

BlockRailTile::BlockRailTile(const std::weak_ptr<World>& world, std::string_view _id) :
  RailTile(world, _id, TileId::RailBlock),
  name{this, "name", id, PropertyFlags::ReadWrite | PropertyFlags::Store},
  state{this, "state", BlockState::Unknown, PropertyFlags::ReadOnly | PropertyFlags::StoreState}
{
  m_data.setSize(1, 5);

  auto w = world.lock();
  const bool editable = w && contains(w->state.value(), WorldState::Edit);

  Attributes::addEnabled(name, editable);
  Attributes::addDisplayName(name, "object:name");
  m_interfaceItems.add(name);
  Attributes::addValues(state, blockStateValues);
  m_interfaceItems.add(state);
}

void BlockRailTile::worldEvent(WorldState state, WorldEvent event)
{
  RailTile::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  name.setAttributeEnabled(editable);
}

void BlockRailTile::setRotate(TileRotate value)
{
  if(value == m_data.rotate())
    return;

  if(value == TileRotate::Deg0 || value == TileRotate::Deg90)
  {
    RailTile::setRotate(value);
    m_data.setSize(m_data.height(), m_data.width());
  }
}
