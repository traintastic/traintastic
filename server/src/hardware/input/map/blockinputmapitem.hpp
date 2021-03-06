/**
 * server/src/hardware/input/map/blockinputmapitem.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INPUT_MAP_BLOCKINPUTMAPITEM_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INPUT_MAP_BLOCKINPUTMAPITEM_HPP

#include "inputmapitem.hpp"
#include "../../../core/objectproperty.hpp"
#include "../input.hpp"
#include "../../../enum/sensortype.hpp"
#include "../../../enum/sensorstate.hpp"

class BlockInputMap;

class BlockInputMapItem : public InputMapItem
{
  CLASS_ID("input_map_item.block")

  private:
    BlockInputMap& m_parent;
    const uint32_t m_itemId;
    boost::signals2::connection m_inputPropertyChanged;
    SensorState m_value;

    void inputPropertyChanged(BaseProperty& property);
    void setValue(SensorState value);

  protected:
    void save(WorldSaver& saver, nlohmann::json& data, nlohmann::json& state) const final;
    void loaded() final;
    void worldEvent(WorldState state, WorldEvent event) final;

  public:
    Property<std::string> name;
    ObjectProperty<Input> input;
    Property<SensorType> type;
    Property<bool> invert;

    BlockInputMapItem(BlockInputMap& parent, uint32_t itemId);

    std::string getObjectId() const final;
    uint32_t itemId() const { return m_itemId; }
    SensorState value() const { return m_value; }
};

#endif
