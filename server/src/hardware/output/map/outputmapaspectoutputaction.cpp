/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2024-2026 Reinder Feenstra
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

#include "outputmapaspectoutputaction.hpp"
#include "../aspectoutput.hpp"
#include "../../../core/attributes.hpp"
#include "../../../core/method.tpp"
#include "../../../world/world.hpp"

OutputMapAspectOutputAction::OutputMapAspectOutputAction(OutputMap& parent_, size_t outputIndex)
  : OutputMapOutputAction(parent_, outputIndex)
  , aspect{this, "aspect", 0, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  const bool editable = contains(world().state.value(), WorldState::Edit);

  Attributes::addEnabled(aspect, editable);
  Attributes::addMinMax(aspect, std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max());
  m_interfaceItems.add(aspect);
}

void OutputMapAspectOutputAction::execute()
{
  aspectOutput().setValue(aspect);
}

TriState OutputMapAspectOutputAction::matchesCurrentOutputState() const
{
  return toTriState(aspectOutput().value.value() == aspect);
}

void OutputMapAspectOutputAction::worldEvent(WorldState state, WorldEvent event)
{
  OutputMapOutputAction::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(aspect, editable);
}

AspectOutput& OutputMapAspectOutputAction::aspectOutput()
{
  assert(dynamic_cast<AspectOutput*>(&output()));
  return static_cast<AspectOutput&>(output());
}

const AspectOutput& OutputMapAspectOutputAction::aspectOutput() const
{
    assert(dynamic_cast<const AspectOutput*>(&output()));
    return static_cast<const AspectOutput&>(output());
}
