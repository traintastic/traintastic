/**
 * server/src/hardware/output/map/outputmapitembase.tpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_OUTPUT_MAP_OUTPUTMAPITEMBASE_TPP
#define TRAINTASTIC_SERVER_HARDWARE_OUTPUT_MAP_OUTPUTMAPITEMBASE_TPP

#include "outputmapitembase.hpp"

#include "../../../core/attributes.hpp"
#include "../../../core/method.tpp"
#include "outputmapoutputaction.hpp"

template<class Key>
OutputMapItemBase<Key>::OutputMapItemBase(Object& map, Key _key)
  : OutputMapItem(map)
  , m_keyValues{{_key}}
  , key{this, "key", _key, PropertyFlags::ReadOnly | PropertyFlags::Store}
  , getOutputAction{*this, "get_output_action",
      [this](uint32_t index)
      {
        return index < outputActions.size() ? outputActions[index] : std::shared_ptr<OutputMapOutputAction>();
      }}
{
  Attributes::addValues(key, m_keyValues);
  m_interfaceItems.add(key);

  m_interfaceItems.add(getOutputAction);
}

template<class Key>
std::string OutputMapItemBase<Key>::getObjectId() const
{
  std::string id{m_map.getObjectId()};
  id.append(".");
  if constexpr(std::is_enum_v<Key>)
  {
    id.append(EnumValues<Key>::value.find(key)->second);
  }
  else if constexpr(std::is_same_v<Key, bool>)
  {
    id.append(key ? "true" : "false");
  }
  else
  {
    static_assert(sizeof(Key) != sizeof(Key));
  }
  return id;
}

#endif // TRAINTASTIC_SERVER_HARDWARE_OUTPUT_MAP_OUTPUTMAPITEMBASE_TPP
