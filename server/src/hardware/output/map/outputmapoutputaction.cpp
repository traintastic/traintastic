/**
 * server/src/hardware/output/map/outputmapoutputaction.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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
#include "../../../core/attributes.hpp"
#include "../../../core/method.tpp"
#include "../../../world/getworld.hpp"

#include <thread> //! @todo remove

OutputMapOutputAction::OutputMapOutputAction(Object& _parent, std::shared_ptr<Output> _output) :
  m_parent{_parent},
  m_output{_output},
  action{this, "action", OutputAction::None, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  const bool editable = contains(getWorld(m_parent).state.value(), WorldState::Edit);

  Attributes::addEnabled(action, editable);
  Attributes::addValues(action, OutputActionValues);
  m_interfaceItems.add(action);
}

std::string OutputMapOutputAction::getObjectId() const
{
  std::string id{m_parent.getObjectId()};
  id.append(".");
  id.append(m_output->getObjectId());
  return id;
}

void OutputMapOutputAction::execute()
{
  switch(action.value())
  {
    case OutputAction::None:
      break;

    case OutputAction::Off:
      m_output->setValue(false);
      break;

    case OutputAction::On:
      m_output->setValue(true);
      break;

    case OutputAction::Pulse:
    {
      //! @todo quick hack, add method pulse to output
      using namespace std::chrono_literals;
      m_output->setValue(true);
      std::this_thread::sleep_for(100ms);
      m_output->setValue(false);
      break;
    }
  }
}

void OutputMapOutputAction::save(WorldSaver& saver, nlohmann::json& data, nlohmann::json& state) const
{
  Object::save(saver, data, state);

  data["output"] = m_output->id.toJSON();
}

void OutputMapOutputAction::worldEvent(WorldState state, WorldEvent event)
{
  Object::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(action, editable);
}
