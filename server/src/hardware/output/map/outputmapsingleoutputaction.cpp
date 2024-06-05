/**
 * server/src/hardware/output/map/outputmapsingleoutputaction.cpp
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

#include "outputmapsingleoutputaction.hpp"
#include "../singleoutput.hpp"
#include "../../../core/attributes.hpp"
#include "../../../core/method.tpp"
#include "../../../world/world.hpp"

#include <thread> // TODO: remove

OutputMapSingleOutputAction::OutputMapSingleOutputAction(OutputMap& parent_, size_t outputIndex)
  : OutputMapOutputAction(parent_, outputIndex)
  , action{this, "action", SingleOutputAction::None, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  const bool editable = contains(world().state.value(), WorldState::Edit);

  Attributes::addEnabled(action, editable);
  Attributes::addValues(action, singleOutputActionValues);
  m_interfaceItems.add(action);
}

void OutputMapSingleOutputAction::execute()
{
  switch(action.value())
  {
    case SingleOutputAction::None:
      break;

    case SingleOutputAction::Off:
      singleOutput().setValue(false);
      break;

    case SingleOutputAction::On:
      singleOutput().setValue(true);
      break;

    case SingleOutputAction::Pulse:
    {
      //! @todo quick hack, add method pulse to output
      using namespace std::chrono_literals;
      singleOutput().setValue(true);
      std::this_thread::sleep_for(100ms);
      singleOutput().setValue(false);
      break;
    }
  }
}

TriState OutputMapSingleOutputAction::matchesCurrentOutputState() const
{
  switch(action.value())
  {
    case SingleOutputAction::None:
      return TriState::Undefined; // None means "any state is ok"

    case SingleOutputAction::Pulse: //TODO: how to detect pulse???
      break;

    case SingleOutputAction::Off:
    {
      if(singleOutput().value.value() == TriState::False)
        return TriState::True;
      break;
    }

    case SingleOutputAction::On:
    {
      if(singleOutput().value.value() == TriState::True)
        return TriState::True;
      break;
    }
  }

  return TriState::False;
}

void OutputMapSingleOutputAction::worldEvent(WorldState state, WorldEvent event)
{
  OutputMapOutputAction::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(action, editable);
}

SingleOutput& OutputMapSingleOutputAction::singleOutput()
{
  assert(dynamic_cast<SingleOutput*>(&output()));
  return static_cast<SingleOutput&>(output());
}

const SingleOutput& OutputMapSingleOutputAction::singleOutput() const
{
    assert(dynamic_cast<const SingleOutput*>(&output()));
    return static_cast<const SingleOutput&>(output());
}
