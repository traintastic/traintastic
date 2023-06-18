/**
 * server/src/board/tile/rail/signal/signalrailtile.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2022 Reinder Feenstra
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
#include "../../../map/signalpath.hpp"
#include "../../../../core/attributes.hpp"
#include "../../../../core/method.tpp"
#include "../../../../core/objectproperty.tpp"
#include "../../../../world/getworld.hpp"
#include "../../../../utils/displayname.hpp"

SignalRailTile::SignalRailTile(World& world, std::string_view _id, TileId tileId) :
  StraightRailTile(world, _id, tileId),
  m_node{*this, 2},
  name{this, "name", std::string(_id), PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly},
  aspect{this, "aspect", SignalAspect::Unknown, PropertyFlags::ReadOnly | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly},
  outputMap{this, "output_map", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject | PropertyFlags::NoScript},
  setAspect{*this, "set_aspect", MethodFlags::ScriptCallable, [this](SignalAspect value) { return doSetAspect(value); }}
{
  const bool editable = contains(m_world.state.value(), WorldState::Edit);

  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);

  Attributes::addObjectEditor(aspect, false);
  // aspect is added by sub class

  Attributes::addDisplayName(outputMap, DisplayName::BoardTile::outputMap);
  m_interfaceItems.add(outputMap);

  Attributes::addObjectEditor(setAspect, false);
  // setAspect is added by sub class
}

SignalRailTile::~SignalRailTile() = default; // default here, so we can use a forward declaration of SignalPath in the header.

void SignalRailTile::worldEvent(WorldState state, WorldEvent event)
{
  StraightRailTile::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(name, editable);
}

bool SignalRailTile::doSetAspect(SignalAspect value)
{
  const auto* values = setAspect.tryGetValuesAttribute(AttributeName::Values);
  assert(values);
  if(!values->contains(static_cast<int64_t>(value)))
    return false;
  (*outputMap)[value]->execute();
  aspect.setValueInternal(value);
  return true;
}
