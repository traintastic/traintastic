/**
 * server/src/hardware/output/map/outputmappairoutputaction.cpp
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

#include "outputmappairoutputaction.hpp"
#include "../pairoutput.hpp"
#include "../../../core/attributes.hpp"
#include "../../../core/method.tpp"
#include "../../../world/world.hpp"

OutputMapPairOutputAction::OutputMapPairOutputAction(OutputMap& parent_, size_t outputIndex)
  : OutputMapOutputAction(parent_, outputIndex)
  , action{this, "action", PairOutputAction::None, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  const bool editable = contains(world().state.value(), WorldState::Edit);

  Attributes::addEnabled(action, editable);
  Attributes::addValues(action, pairOutputActionValues);
  m_interfaceItems.add(action);
}

void OutputMapPairOutputAction::execute()
{
  switch(action.value())
  {
    case PairOutputAction::None:
      break;

    case PairOutputAction::First:
      pairOutput().setValue(OutputPairValue::First);
      break;

    case PairOutputAction::Second:
      pairOutput().setValue(OutputPairValue::Second);
      break;
  }
}

TriState OutputMapPairOutputAction::matchesCurrentOutputState() const
{
  switch(action.value())
  {
    case PairOutputAction::None:
      return TriState::Undefined; // None means "any state is ok"

    case PairOutputAction::First:
    {
      if(pairOutput().value.value() == OutputPairValue::First)
        return TriState::True;
      break;
    }

    case PairOutputAction::Second:
    {
      if(pairOutput().value.value() == OutputPairValue::Second)
        return TriState::True;
      break;
    }
  }

  return TriState::False;
}

void OutputMapPairOutputAction::worldEvent(WorldState state, WorldEvent event)
{
  OutputMapOutputAction::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(action, editable);
}

PairOutput& OutputMapPairOutputAction::pairOutput()
{
  assert(dynamic_cast<PairOutput*>(&output()));
  return static_cast<PairOutput&>(output());
}

const PairOutput& OutputMapPairOutputAction::pairOutput() const
{
    assert(dynamic_cast<const PairOutput*>(&output()));
    return static_cast<const PairOutput&>(output());
}
