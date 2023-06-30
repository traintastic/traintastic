/**
 * server/src/hardware/output/map/outputmap.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023 Reinder Feenstra
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
#include "../../../core/method.tpp"
#include "../../../world/getworld.hpp"
#include "../../../world/worldloader.hpp"
#include "../../../world/worldsaver.hpp"
#include "../../../utils/displayname.hpp"

OutputMap::OutputMap(Object& _parent, std::string_view parentPropertyName) :
  SubObject(_parent, parentPropertyName),
  addOutput{*this, "add_output",
    [this](std::shared_ptr<Output> output)
    {
      if(std::find(m_outputs.begin(), m_outputs.end(), output) != m_outputs.end())
        return;

      output->controllers.appendInternal(parent().shared_from_this());
      m_destroyingConnections.emplace(output, output->onDestroying.connect(
        [this](Object& object)
        {
          removeOutput(object.shared_ptr<Output>());
        }));

      outputAdded(output);
      m_outputs.emplace_back(std::move(output));
      outputsChanged(*this);
    }},
  removeOutput{*this, "remove_output",
    [this](const std::shared_ptr<Output>& output)
    {
      if(auto it = std::find(m_outputs.begin(), m_outputs.end(), output); it != m_outputs.end())
      {
        output->controllers.removeInternal(parent().shared_from_this());
        if(auto con = m_destroyingConnections.find(output); con != m_destroyingConnections.end())
        {
          con->second.disconnect();
          m_destroyingConnections.erase(con);
        }

        m_outputs.erase(it);
        outputRemoved(output);
        outputsChanged(*this);
      }
    }}
{
  auto& world = getWorld(&_parent);
  const bool editable = contains(world.state.value(), WorldState::Edit);

  Attributes::addDisplayName(addOutput, DisplayName::List::add);
  Attributes::addEnabled(addOutput, editable);
  Attributes::addObjectList(addOutput, world.outputs);
  m_interfaceItems.add(addOutput);

  Attributes::addDisplayName(removeOutput, DisplayName::List::remove);
  Attributes::addEnabled(removeOutput, editable);
  m_interfaceItems.add(removeOutput);
}

void OutputMap::destroying()
{
  for(auto& it : m_destroyingConnections)
    it.second.disconnect();
  SubObject::destroying();
}

void OutputMap::load(WorldLoader& loader, const nlohmann::json& data)
{
  SubObject::load(loader, data);
  nlohmann::json outputs = data.value("outputs", nlohmann::json::array());
  for(auto& [_, id] : outputs.items())
  {
    static_cast<void>(_); // silence unused warning
    if(auto output = std::dynamic_pointer_cast<Output>(loader.getObject(id.get<std::string_view>())))
      addOutput(std::move(output));
  }
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
