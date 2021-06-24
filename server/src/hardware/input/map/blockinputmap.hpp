/**
 * server/src/hardware/input/map/blockinputmap.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INPUT_MAP_BLOCKINPUTMAP_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INPUT_MAP_BLOCKINPUTMAP_HPP

#include "inputmap.hpp"
#include "blockinputmapitem.hpp"
#include "../../../core/method.hpp"
#include "../../../core/objectvectorproperty.hpp"

class BlockInputMap : public InputMap
{
  CLASS_ID("input_map.block")

  private:
    unsigned int getItemId() const;

  protected:
    void load(WorldLoader& loader, const nlohmann::json& data) final;
    void worldEvent(WorldState state, WorldEvent event) final;

  public:
    ObjectVectorProperty<BlockInputMapItem> items;
    Method<void()> add;
    Method<void(const std::shared_ptr<BlockInputMapItem>& item)> remove;
    Method<void(const std::shared_ptr<BlockInputMapItem>& item)> moveUp;
    Method<void(const std::shared_ptr<BlockInputMapItem>& item)> moveDown;

    BlockInputMap(Object& _parent, const std::string& parentPropertyName);

    void itemValueChanged(BlockInputMapItem& item);
};

#endif
