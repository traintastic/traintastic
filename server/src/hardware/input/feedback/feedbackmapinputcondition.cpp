/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#include "feedbackmapinputcondition.hpp"
#include "feedbackmap.hpp"
#include "../input.hpp"
#include "../../../core/attributes.hpp"
#include "../../../utils/tohex.hpp"
#include "../../../world/getworld.hpp"
#include "../../../world/world.hpp"

FeedbackMapInputCondition::FeedbackMapInputCondition(FeedbackMap& feedbackMap, size_t inputIndex)
  : condition{this, "condition", InputCondition::DontCare, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , m_feedbackMap{feedbackMap}
  , m_inputIndex{inputIndex}
{
  const auto& world = getWorld(feedbackMap);
  const bool editable = contains(world.state, WorldState::Edit);

  Attributes::addEnabled(condition, editable);
  Attributes::addValues(condition, inputConditionValues);
  m_interfaceItems.add(condition);
}

std::string FeedbackMapInputCondition::getObjectId() const
{
  std::string id{m_feedbackMap.getObjectId()};
  id.append(".condition_");
  id.append(toHex(reinterpret_cast<uintptr_t>(this)));
  return id;
}

bool FeedbackMapInputCondition::matches() const
{
  switch(condition.value())
  {
    case InputCondition::DontCare:
      return true;

    case InputCondition::Off:
      return m_feedbackMap.input(m_inputIndex)->value == TriState::False;

    case InputCondition::On:
      return m_feedbackMap.input(m_inputIndex)->value == TriState::True;
  }
  assert(false);
  return false;
}

void FeedbackMapInputCondition::worldEvent(WorldState state, WorldEvent event)
{
  Object::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(condition, editable);
}
