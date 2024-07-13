/**
 * server/src/board/tile/rail/decouplerrailtile.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2024 Reinder Feenstra
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

#include "decouplerrailtile.hpp"
#include "../../../core/method.tpp"
#include "../../../core/objectproperty.tpp"
#include "../../../world/world.hpp"
#include "../../../core/attributes.hpp"
#include "../../../hardware/output/outputcontroller.hpp"
#include "../../../utils/displayname.hpp"

CREATE_IMPL(DecouplerRailTile)

DecouplerRailTile::DecouplerRailTile(World& world, std::string_view _id)
  : StraightRailTile(world, _id, TileId::RailDecoupler)
  , m_node{*this, 2}
  , name{this, "name", id, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly}
  , state{this, "state", DecouplerState::Deactivated, PropertyFlags::ReadOnly | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly}
  , outputMap{this, "output_map", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject | PropertyFlags::NoScript}
  , activate{*this, "activate",
      [this]()
      {
        setState(DecouplerState::Activated);
      }}
  , deactivate{*this, "deactivate",
      [this]()
      {
        setState(DecouplerState::Deactivated);
      }}
{
  outputMap.setValueInternal(std::make_shared<DecouplerOutputMap>(*this, outputMap.name()));

  const bool editable = contains(m_world.state.value(), WorldState::Edit);

  Attributes::addEnabled(name, editable);
  Attributes::addDisplayName(name, DisplayName::Object::name);
  m_interfaceItems.add(name);

  Attributes::addObjectEditor(state, false);
  Attributes::addValues(state, decouplerStateValues);
  m_interfaceItems.add(state);

  Attributes::addDisplayName(outputMap, DisplayName::BoardTile::outputMap);
  m_interfaceItems.add(outputMap);

  Attributes::addObjectEditor(activate, false);
  m_interfaceItems.add(activate);

  Attributes::addObjectEditor(deactivate, false);
  m_interfaceItems.add(deactivate);

  outputMap->onOutputStateMatchFound.connect([this](DecouplerState value)
    {
      setState(value, true);
    });
}

void DecouplerRailTile::destroying()
{
  outputMap->parentObject.setValueInternal(nullptr);
  StraightRailTile::addToWorld();
}

void DecouplerRailTile::addToWorld()
{
  outputMap->parentObject.setValueInternal(shared_from_this());
  StraightRailTile::addToWorld();
}

void DecouplerRailTile::worldEvent(WorldState worldState, WorldEvent worldEvent)
{
  StraightRailTile::worldEvent(worldState, worldEvent);

  const bool editable = contains(worldState, WorldState::Edit);

  Attributes::setEnabled(name, editable);
}

void DecouplerRailTile::setState(DecouplerState value, bool skipAction)
{
  if(state != value)
  {
    if(!skipAction)
      (*outputMap)[value]->execute();
    state.setValueInternal(value);
  }
}
