/**
 * server/src/zone/blockzonelist.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_ZONE_BLOCKZONELIST_HPP
#define TRAINTASTIC_SERVER_ZONE_BLOCKZONELIST_HPP

#include "../core/method.hpp"
#include "../core/objectlist.hpp"
#include "zone.hpp"

class BlockRailTile;
class Zone;

class BlockZoneList : public ObjectList<Zone>
{
  CLASS_ID("list.zone_block")

private:
  inline BlockRailTile& block();

protected:
  void worldEvent(WorldState worldState, WorldEvent worldEvent) override;
  bool isListedProperty(std::string_view name) final;

public:
  Method<void(const std::shared_ptr<Zone>&)> add;
  Method<void(const std::shared_ptr<Zone>&)> remove;

  BlockZoneList(BlockRailTile& block_, std::string_view parentPropertyName);

  TableModelPtr getModel() final;
};

#endif
