/**
 * server/src/hardware/output/map/outputmapitem.cpp
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

#include "outputmapitem.hpp"
#include "outputmapoutputaction.hpp"
#include "../../../world/worldloader.hpp"
#include "../../../world/worldsaver.hpp"

OutputMapItem::OutputMapItem(Object& map) :
  m_map{map}
{
}

const OutputMapItem::OutputActions& OutputMapItem::outputActions() const
{
  return m_outputActions;
}

void OutputMapItem::execute()
{
  for(const auto& action : m_outputActions)
    action->execute();
}

void OutputMapItem::load(WorldLoader& loader, const nlohmann::json& data)
{
  Object::load(loader, data);

  nlohmann::json actions = data.value("output_actions", nlohmann::json::array());
  for(auto& [_, action] : actions.items())
  {
    static_cast<void>(_); // silence unused warning
    auto output = std::dynamic_pointer_cast<Output>(loader.getObject(action.value("output", "")));
    auto it = std::find_if(m_outputActions.begin(), m_outputActions.end(),
      [&output](const auto& item)
      {
        return output == item->output();
      });

    if(it != m_outputActions.end())
      (**it).load(loader, action);
  }
}

void OutputMapItem::save(WorldSaver& saver, nlohmann::json& data, nlohmann::json& state) const
{
  Object::save(saver, data, state);

  nlohmann::json outputActions = nlohmann::json::array();
  for(const auto& outputAction : m_outputActions)
    outputActions.emplace_back(saver.saveObject(outputAction));
  data["output_actions"] = outputActions;
}

void OutputMapItem::worldEvent(WorldState state, WorldEvent event)
{
  Object::worldEvent(state, event);
  for(auto& action : m_outputActions)
    action->worldEvent(state, event);
}

void OutputMapItem::addOutput(const std::shared_ptr<Output>& output)
{
  m_outputActions.emplace_back(new OutputMapOutputAction(m_map, output));
}

void OutputMapItem::removeOutput(const std::shared_ptr<Output>& output)
{
  auto it = std::find_if(m_outputActions.begin(), m_outputActions.end(), [output](const auto& item){ return item->output() == output; });
  if(it != m_outputActions.end())
  {
    (*it)->destroy();
    m_outputActions.erase(it);
  }
}
