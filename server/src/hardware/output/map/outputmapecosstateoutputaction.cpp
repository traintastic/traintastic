/**
 * server/src/hardware/output/map/outputmapecosstateoutputaction.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#include "outputmapecosstateoutputaction.hpp"
#include "../ecosstateoutput.hpp"
#include "../../../core/attributes.hpp"
#include "../../../core/method.tpp"
#include "../../../world/world.hpp"

OutputMapECoSStateOutputAction::OutputMapECoSStateOutputAction(OutputMap& parent_, size_t outputIndex)
  : OutputMapOutputAction(parent_, outputIndex)
  , state{this, "state", 0, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  const bool editable = contains(world().state.value(), WorldState::Edit);

  Attributes::addEnabled(state, editable);
  m_interfaceItems.add(state);
}

void OutputMapECoSStateOutputAction::execute()
{
  ecosStateOutput().setValue(state);
}

TriState OutputMapECoSStateOutputAction::matchesCurrentOutputState() const
{
  return toTriState(ecosStateOutput().value.value() == state);
}

void OutputMapECoSStateOutputAction::worldEvent(WorldState worldState, WorldEvent event)
{
  OutputMapOutputAction::worldEvent(worldState, event);

  const bool editable = contains(worldState, WorldState::Edit);

  Attributes::setEnabled(state, editable);
}

ECoSStateOutput& OutputMapECoSStateOutputAction::ecosStateOutput()
{
  assert(dynamic_cast<ECoSStateOutput*>(&output()));
  return static_cast<ECoSStateOutput&>(output());
}

const ECoSStateOutput& OutputMapECoSStateOutputAction::ecosStateOutput() const
{
    assert(dynamic_cast<const ECoSStateOutput*>(&output()));
    return static_cast<const ECoSStateOutput&>(output());
}
