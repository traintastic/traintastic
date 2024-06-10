/**
 * server/src/hardware/output/map/outputmapbase.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_OUTPUT_MAP_OUTPUTMAPBASE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_OUTPUT_MAP_OUTPUTMAPBASE_HPP

#include "outputmap.hpp"
#include <initializer_list>
#include "../../../core/event.hpp"

template<class Key, class Value>
class OutputMapBase : public OutputMap
{
  public:
    using DefaultOutputActionGetter = std::optional<OutputActionValue>(*)(Key, OutputType, size_t);

  protected:
    DefaultOutputActionGetter m_defaultOutputActionGetter;

    void load(WorldLoader& loader, const nlohmann::json& data) override
    {
      OutputMap::load(loader, data);

      nlohmann::json itemsData = data.value("items", nlohmann::json::array());
      for(auto& [_, item] : itemsData.items())
      {
        static_cast<void>(_); // silence unused warning
        if(auto object = operator[](to<Key>(item["key"]))) /*[[likely]]*/
        {
          object->load(loader, item);
        }
      }
    }

    std::optional<OutputActionValue> getDefaultOutputActionValue(const OutputMapItem& item, OutputType outputType, size_t outputIndex) override
    {
      return m_defaultOutputActionGetter(static_cast<const Value&>(item).key.value(), outputType, outputIndex);
    }

  public:
    Event<Key> onOutputStateMatchFound;

    OutputMapBase(Object& _parent, std::string_view parentPropertyName, std::initializer_list<Key> keys, DefaultOutputActionGetter defaultOutputActionGetter) :
      OutputMap(_parent, parentPropertyName)
      , m_defaultOutputActionGetter{defaultOutputActionGetter}
      , onOutputStateMatchFound{*this, "on_match_found", EventFlags::Scriptable}
    {
      assert(m_defaultOutputActionGetter);
      for(auto k : keys)
      {
        items.appendInternal(std::make_shared<Value>(*this, k));
      }
    }

    const std::shared_ptr<Value> operator [](Key key) const
    {
      for(auto item : items)
      {
        if(static_cast<Value&>(*item).key == key)
        {
          return std::static_pointer_cast<Value>(item);
        }
      }
      assert(false);
      return {};
    }

    void updateStateFromOutput() override
    {
      int match = getMatchingActionOnCurrentState();
      if(match < 0)
        return; // No match found

      auto item = std::static_pointer_cast<Value>(items[match]);
      fireEvent(onOutputStateMatchFound, item->key.value());
    }
};

#endif
