/**
 * server/src/hardware/output/map/outputmapoutputaction.cpp
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

#include "outputmapoutputaction.hpp"
#include "outputmap.hpp"
#include "../../../world/getworld.hpp"
#include "../../../utils/tohex.hpp"

OutputMapOutputAction::OutputMapOutputAction(OutputMap& _parent, size_t outputIndex) :
  m_parent{_parent},
  m_outputIndex{outputIndex}
{
}

std::string OutputMapOutputAction::getObjectId() const
{
  std::string id{m_parent.getObjectId()};
  id.append(".action_");
  id.append(toHex(reinterpret_cast<uintptr_t>(this)));
  return id;
}

World& OutputMapOutputAction::world()
{
  return getWorld(m_parent);
}

Output& OutputMapOutputAction::output()
{
  return *m_parent.output(m_outputIndex);
}

const Output& OutputMapOutputAction::output() const
{
    return *m_parent.output(m_outputIndex);
}
