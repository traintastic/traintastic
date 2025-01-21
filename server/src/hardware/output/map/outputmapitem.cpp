/**
 * server/src/hardware/output/map/outputmapitem.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2024 Reinder Feenstra
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

#include "outputmapitem.hpp"
#include "outputmapoutputaction.hpp"
#include "../../../core/objectvectorproperty.tpp"
#include "../../../world/worldloader.hpp"
#include "../../../world/worldsaver.hpp"

OutputMapItem::OutputMapItem(Object& map) :
  m_map{map}
  , outputActions{*this, "output_actions", {}, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::Store | PropertyFlags::NoScript}
{
  m_interfaceItems.add(outputActions);
}

void OutputMapItem::execute()
{
  for(const auto& action : outputActions)
  {
    action->execute();
  }
}

OutputMapItem::MatchResult OutputMapItem::matchesCurrentOutputState() const
{
  bool atLeastOneMatch = false;
  bool atLeastOneDifference = false;
  bool atLeastOneWildcard = false;

  for(const auto& action : outputActions)
  {
    TriState match = action->matchesCurrentOutputState();
    switch (match)
    {
    case TriState::Undefined:
      atLeastOneWildcard = true;
      break;
    case TriState::False:
      atLeastOneDifference = true;
      break;
    case TriState::True:
      atLeastOneMatch = true;
      break;
    }

    // This state could be the coorect one but output feedback is not yet arrived
    // We will re-check when we receive output feedback from command station
    if(atLeastOneDifference && (atLeastOneMatch || atLeastOneWildcard))
      return MatchResult::PartialMatch;
  }

  if(atLeastOneWildcard)
    return MatchResult::WildcardMatch;
  if(atLeastOneMatch)
    return MatchResult::FullMatch;
  return MatchResult::NoMatch;
}

void OutputMapItem::worldEvent(WorldState state, WorldEvent event)
{
  Object::worldEvent(state, event);
  for(const auto& action : outputActions)
  {
    action->worldEvent(state, event);
  }
}
