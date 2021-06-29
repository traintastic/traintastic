/**
 * server/src/board/tile/rail/signal/signalrailtile.cpp
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

#include "signalrailtile.hpp"
#include "../../../../core/attributes.hpp"
#include "../../../../world/getworld.hpp"
#include "../../../../utils/displayname.hpp"

SignalRailTile::SignalRailTile(const std::weak_ptr<World>& world, std::string_view _id, TileId tileId) :
  StraightRailTile(world, _id, tileId),
  name{this, "name", std::string(_id), PropertyFlags::ReadWrite | PropertyFlags::Store},
  aspect{this, "aspect", SignalAspect::Unknown, PropertyFlags::ReadWrite | PropertyFlags::StoreState,
    [this](SignalAspect value)
    {
      (*outputMap)[value]->execute();
    }},
  outputMap{this, "output_map", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject},
  nextAspect{*this, "next_aspect", [this](bool reverse){ doNextAspect(reverse); }}
{
  auto w = world.lock();
  const bool editable = w && contains(w->state.value(), WorldState::Edit);

  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);
  Attributes::addObjectEditor(aspect, false);
  m_interfaceItems.add(outputMap);
  Attributes::addObjectEditor(nextAspect, false);
  m_interfaceItems.add(nextAspect);
}

void SignalRailTile::worldEvent(WorldState state, WorldEvent event)
{
  StraightRailTile::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  name.setAttributeEnabled(editable);
}
