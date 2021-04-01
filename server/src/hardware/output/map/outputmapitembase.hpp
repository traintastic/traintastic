/**
 * server/src/hardware/output/map/outputmapitembase.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_OUTPUT_MAP_OUTPUTMAPITEMBASE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_OUTPUT_MAP_OUTPUTMAPITEMBASE_HPP

#include "outputmapitem.hpp"
#include <vector>
#include "../../../core/attributes.hpp"
#include "../../../core/method.hpp"
#include "../output.hpp"
#include "outputmapoutputaction.hpp"

template<class Key>
class OutputMapItemBase : public OutputMapItem
{
  template<class AKey, class AValue>
  friend class OutputMapBase;

  private:
    const std::array<Key, 1> m_keyValues;

  public:
    Property<Key> key;
    Property<bool> use;
    Method<std::shared_ptr<OutputMapOutputAction> (const std::shared_ptr<Output>&)> getOutputAction;

    OutputMapItemBase(Object& map, Key _key) :
      OutputMapItem(map),
      m_keyValues{{_key}},
      key{this, "key", _key, PropertyFlags::ReadOnly | PropertyFlags::Store},
      use{this, "use", true, PropertyFlags::ReadWrite | PropertyFlags::Store},
      getOutputAction{*this, "get_output_action",
        [this](const std::shared_ptr<Output>& output)
        {
          auto it = std::find_if(m_outputActions.begin(), m_outputActions.end(),
            [output](const auto& item)
            {
              return item->output() == output;
            });
          return it != m_outputActions.end() ? *it : std::shared_ptr<OutputMapOutputAction>();
        }}
    {
      Attributes::addValues(key, m_keyValues);
      m_interfaceItems.add(key);
      Attributes::addEnabled(use, false);
      m_interfaceItems.add(use);
      m_interfaceItems.add(getOutputAction);
    }

    std::string getObjectId() const final
    {
      std::string id{m_map.getObjectId()};
      id.append(".");
      id.append(EnumValues<Key>::value.find(key)->second);
      return id;
    }
};

#endif
