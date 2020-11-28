/**
 * server/src/board/tile/rail/sensorrailtile.cpp
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

#include "sensorrailtile.hpp"
#include "../../../world/world.hpp"
#include "../../../core/attributes.hpp"

SensorRailTile::SensorRailTile(const std::weak_ptr<World>& world, std::string_view _id) :
  StraightRailTile(world, _id, TileId::RailSensor),
  input{this, "input", nullptr, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  auto w = world.lock();
  const bool editable = w && contains(w->state.value(), WorldState::Edit);

  Attributes::addEnabled(input, editable);
  Attributes::addObjectList(input, w->inputs);
  m_interfaceItems.add(input);
}

void SensorRailTile::worldEvent(WorldState state, WorldEvent event)
{
  StraightRailTile::worldEvent(state, event);

  input.setAttributeEnabled(contains(state, WorldState::Edit));
}
