/**
 * server/src/board/tile/rail/directioncontrolrailtile.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2025 Reinder Feenstra
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

#include "directioncontrolrailtile.hpp"
#include "../../../world/world.hpp"
#include "../../../core/method.tpp"
#include "../../../core/attributes.hpp"
#include "../../../utils/sensor.hpp"
#include "../../../utils/category.hpp"
#include "../../../utils/displayname.hpp"

CREATE_IMPL(DirectionControlRailTile)

DirectionControlRailTile::DirectionControlRailTile(World& world, std::string_view _id)
  : StraightRailTile(world, _id, TileId::RailDirectionControl)
  , m_node{*this, 2}
  , m_reservedState(DirectionControlState::None)
  , name{this, "name", id, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly}
  , useNone{this, "use_none", true, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly,
      [this](const bool /*value*/)
      {
        updateEnabled();
        updateStateValues();
      }}
  , useAtoB{this, "use_a_to_b", true, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly,
      [this](const bool /*value*/)
      {
        updateEnabled();
        updateStateValues();
      }}
  , useBtoA{this, "use_b_to_a", true, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly,
      [this](const bool /*value*/)
      {
        updateEnabled();
        updateStateValues();
      }}
  , useBoth{this, "use_both", true, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly,
      [this](const bool /*value*/)
      {
        updateEnabled();
        updateStateValues();
      }}
  , state{this, "state", DirectionControlState::Both, PropertyFlags::ReadOnly | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly}
  , setState{*this, "set_state", MethodFlags::ScriptCallable,
      [this](DirectionControlState newState)
      {
        if(reservedState() && newState != m_reservedState)
        {
          // Direction control is currently locked by reserved path
          // Allow setting to Both directions unless reserved direction is None
          if(m_reservedState == DirectionControlState::None || newState != DirectionControlState::Both)
            return false;
        }

        const auto& states = setState.getVectorAttribute<DirectionControlState>(AttributeName::Values);
        if(std::find(states.begin(), states.end(), newState) == states.end())
          return false;
        state.setValueInternal(newState);
        stateChanged(*this, newState);
        return true;
      }}
  , toggle{*this, "toggle", MethodFlags::Internal | MethodFlags::ScriptCallable,
      [this]()
      {
        switch(state.value())
        {
          case DirectionControlState::AtoB:
            return setState(DirectionControlState::BtoA);

          case DirectionControlState::BtoA:
            return setState(DirectionControlState::AtoB);

          case DirectionControlState::None:
          case DirectionControlState::Both:
            return false;
        }
        assert(false);
        return false;
      }}
{
  const bool editable = contains(m_world.state.value(), WorldState::Edit);

  Attributes::addEnabled(name, editable);
  Attributes::addDisplayName(name, DisplayName::Object::name);
  m_interfaceItems.add(name);

  Attributes::addCategory(useNone, Category::options);
  Attributes::addEnabled(useNone, false);
  m_interfaceItems.add(useNone);

  Attributes::addCategory(useAtoB, Category::options);
  Attributes::addEnabled(useAtoB, false);
  m_interfaceItems.add(useAtoB);

  Attributes::addCategory(useBtoA, Category::options);
  Attributes::addEnabled(useBtoA, false);
  m_interfaceItems.add(useBtoA);

  Attributes::addCategory(useBoth, Category::options);
  Attributes::addEnabled(useBoth, false);
  m_interfaceItems.add(useBoth);

  Attributes::addObjectEditor(state, false);
  Attributes::addValues(state, std::vector<DirectionControlState>{});
  m_interfaceItems.add(state);

  Attributes::addObjectEditor(setState, false);
  Attributes::addValues(setState, std::vector<DirectionControlState>{});
  m_interfaceItems.add(setState);

  m_interfaceItems.add(toggle);

  updateEnabled();
  updateStateValues();
}

bool DirectionControlRailTile::reserve(DirectionControlState directionControlState, bool dryRun)
{
  if(reservedState() || (state != directionControlState && state != DirectionControlState::Both))
  {
    return false;
  }

  if(!dryRun)
  {
    m_reservedState = directionControlState;
    StraightRailTile::reserve();
  }

  return true;
}

void DirectionControlRailTile::loaded()
{
  StraightRailTile::loaded();

  updateEnabled();
  updateStateValues();
}

void DirectionControlRailTile::worldEvent(WorldState worldState, WorldEvent worldEvent)
{
  StraightRailTile::worldEvent(worldState, worldEvent);

  const bool editable = contains(worldState, WorldState::Edit);

  Attributes::setEnabled(name, editable);

  updateEnabled();
}

void DirectionControlRailTile::updateEnabled()
{
  const bool editable = contains(m_world.state.value(), WorldState::Edit) && !contains(m_world.state.value(), WorldState::Run);
  const bool moreThanTwoInUse =
    [this]()
    {
      size_t n = 0;
      for(const auto use : std::initializer_list<bool>{useNone, useAtoB, useBtoA, useBoth})
        if(use)
          n++;
      return n > 2;
    }();

  Attributes::setEnabled(useNone, editable && (!useNone || moreThanTwoInUse));
  Attributes::setEnabled(useAtoB, editable && (!useAtoB || moreThanTwoInUse));
  Attributes::setEnabled(useBtoA, editable && (!useBtoA || moreThanTwoInUse));
  Attributes::setEnabled(useBoth, editable && (!useBoth || moreThanTwoInUse));
}

void DirectionControlRailTile::updateStateValues()
{
  std::vector<DirectionControlState> states;
  if(useNone)
    states.emplace_back(DirectionControlState::None);
  if(useAtoB)
    states.emplace_back(DirectionControlState::AtoB);
  if(useBtoA)
    states.emplace_back(DirectionControlState::BtoA);
  if(useBoth)
    states.emplace_back(DirectionControlState::Both);

  if(std::find(states.begin(), states.end(), state.value()) == states.end() && !states.empty())
    state.setValueInternal(states.front());

  Attributes::setValues(state, states);
  Attributes::setValues(setState, std::move(states));
}
