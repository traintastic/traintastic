/**
 * server/src/board/tile/rail/turnout/turnoutrailtile.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2024 Reinder Feenstra
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
#include "../../../../core/objectproperty.tpp"
#include "../../../../core/attributes.hpp"
#include "../../../../core/method.tpp"
#include "../../../../world/world.hpp"
#include "../../../../utils/displayname.hpp"

TurnoutRailTile::TurnoutRailTile(World& world, std::string_view _id, TileId tileId_, size_t connectors) :
  RailTile(world, _id, tileId_),
  m_node{*this, connectors},
  name{this, "name", std::string(_id), PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly},
  position{this, "position", TurnoutPosition::Unknown, PropertyFlags::ReadWrite | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly},
  outputMap{this, "output_map", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject | PropertyFlags::NoScript},
  setPosition{*this, "set_position", MethodFlags::ScriptCallable, [this](TurnoutPosition value) { return doSetPosition(value); }}
{
  assert(isRailTurnout(tileId_));

  const bool editable = contains(m_world.state.value(), WorldState::Edit);

  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);

  Attributes::addDisplayName(position, DisplayName::BoardTile::Turnout::position);
  Attributes::addObjectEditor(position, false);
  // position is added by sub class

  Attributes::addDisplayName(outputMap, DisplayName::BoardTile::outputMap);
  m_interfaceItems.add(outputMap);

  Attributes::addObjectEditor(setPosition, false);
  // setPosition is added by sub class
}

bool TurnoutRailTile::reserve(TurnoutPosition turnoutPosition, bool dryRun)
{
  if(!isValidPosition(turnoutPosition))
  {
    return false;
  }
  if(!dryRun)
  {
    if(!doSetPosition(turnoutPosition)) /*[[unlikely]]*/
    {
      return false;
    }

    RailTile::setReservedState(static_cast<uint8_t>(turnoutPosition));
  }
  return true;
}

bool TurnoutRailTile::release(bool dryRun)
{
  //! \todo check occupancy sensor, once supported

  if(!dryRun)
  {
    RailTile::release();
  }
  return true;
}

void TurnoutRailTile::destroying()
{
  outputMap->parentObject.setValueInternal(nullptr);
  RailTile::addToWorld();
}

void TurnoutRailTile::addToWorld()
{
  outputMap->parentObject.setValueInternal(shared_from_this());
  RailTile::addToWorld();
}

void TurnoutRailTile::worldEvent(WorldState state, WorldEvent event)
{
  RailTile::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(name, editable);
}

bool TurnoutRailTile::isValidPosition(TurnoutPosition value)
{
  const auto* values = setPosition.tryGetValuesAttribute(AttributeName::Values);
  assert(values);
  return values->contains(static_cast<int64_t>(value));
}

bool TurnoutRailTile::doSetPosition(TurnoutPosition value, bool skipAction)
{
  if(!isValidPosition(value))
  {
    return false;
  }
  if(!skipAction)
    (*outputMap)[value]->execute();
  position.setValueInternal(value);
  positionChanged(*this, value);
  return true;
}

void TurnoutRailTile::connectOutputMap()
{
  outputMap->onOutputStateMatchFound.connect([this](TurnoutPosition pos)
    {
      doSetPosition(pos, true);
    });

  //TODO: disconnect somewhere?
}
