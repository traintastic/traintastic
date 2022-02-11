/**
 * server/src/hardware/output/map/outputmapbase.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_OUTPUT_MAP_OUTPUTMAPBASE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_OUTPUT_MAP_OUTPUTMAPBASE_HPP

#include "outputmap.hpp"
#include <unordered_map>
#include <initializer_list>
#include "outputmapoutputaction.hpp"

template<class Key, class Value>
class OutputMapBase : public OutputMap
{
  public:
    using ItemMap = std::unordered_map<Key, std::shared_ptr<Value>>;

  protected:
    const std::vector<Key> m_keys;
    ItemMap m_items;

    void load(WorldLoader& loader, const nlohmann::json& data) override
    {
      OutputMap::load(loader, data);
      nlohmann::json items = data.value("items", nlohmann::json::array());
      for(auto& [_, item] : items.items())
      {
        static_cast<void>(_); // silence unused warning
        Key k = to<Key>(item["key"]);
        m_items[k]->load(loader, item);
      }
    }

    void worldEvent(WorldState state, WorldEvent event) override
    {
      OutputMap::worldEvent(state, event);
      for(const auto& it : m_items)
        it.second->worldEvent(state, event);
    }

    void outputAdded(const std::shared_ptr<Output>& output) final
    {
      for(const auto& it : m_items)
        it.second->addOutput(output);
    }

    void outputRemoved(const std::shared_ptr<Output>& output) final
    {
      for(const auto& it : m_items)
        it.second->removeOutput(output);
    }

  public:
    OutputMapBase(Object& _parent, std::string_view parentPropertyName, std::initializer_list<Key> keys) :
      OutputMap(_parent, parentPropertyName),
      m_keys{keys}
    {
      for(auto k : keys)
        m_items.emplace(k, std::make_shared<Value>(*this, k));
    }

    const std::shared_ptr<Value> operator [](Key key) const
    {
      return m_items.at(key);
    }

    Items items() const final
    {
      Items items;
      items.reserve(m_keys.size());
      for(auto k : m_keys)
        items.emplace_back(m_items.at(k));
      return items;
    }
};

#endif
