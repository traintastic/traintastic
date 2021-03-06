/**
 * server/src/hardware/output/map/outputmap.cpp
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

#include "outputmap.hpp"
#include <cassert>
#include "outputmapitem.hpp"
#include "../../../core/attributes.hpp"
#include "../../../world/getworld.hpp"
#include "../../../world/worldloader.hpp"
#include "../../../world/worldsaver.hpp"

OutputMap::OutputMap(Object& _parent, const std::string& parentPropertyName) :
  SubObject(_parent, parentPropertyName),
  addOutput{*this, "add_output",
    [this](std::shared_ptr<Output> output)
    {
      if(std::find(m_outputs.begin(), m_outputs.end(), output) != m_outputs.end())
        return;
      outputAdded(output);
      m_outputs.emplace_back(std::move(output));
      outputsChanged(*this);
    }},
  removeOutput{*this, "remove_output",
    [this](const std::shared_ptr<Output>& output)
    {
      if(auto it = std::find(m_outputs.begin(), m_outputs.end(), output); it != m_outputs.end())
      {
        m_outputs.erase(it);
        outputRemoved(output);
        outputsChanged(*this);
      }
    }}
{
  auto w = getWorld(&_parent);
  assert(w);

  const bool editable = contains(w->state.value(), WorldState::Edit);

  Attributes::addEnabled(addOutput, editable);
  Attributes::addObjectList(addOutput, w->outputs);
  m_interfaceItems.add(addOutput);
  Attributes::addEnabled(removeOutput, editable);
  m_interfaceItems.add(removeOutput);
}

void OutputMap::load(WorldLoader& loader, const nlohmann::json& data)
{
  SubObject::load(loader, data);
  nlohmann::json outputs = data.value("outputs", nlohmann::json::array());
  for(auto& [_, id] : outputs.items())
    if(auto output = std::dynamic_pointer_cast<Output>(loader.getObject(id)))
      addOutput(std::move(output));
}

void OutputMap::save(WorldSaver& saver, nlohmann::json& data, nlohmann::json& state) const
{
  SubObject::save(saver, data, state);

  nlohmann::json items = nlohmann::json::array();
  for(const auto& item : this->items())
    items.emplace_back(saver.saveObject(item));
  data["items"] = items;

  nlohmann::json outputs = nlohmann::json::array();
  for(const auto& item : m_outputs)
    outputs.emplace_back(item->id.toJSON());
  data["outputs"] = outputs;
}

void OutputMap::worldEvent(WorldState state, WorldEvent event)
{
  SubObject::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(addOutput, editable);
  Attributes::setEnabled(removeOutput, editable);
}
