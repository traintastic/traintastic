/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_ROUTE_TRAINROUTEENTRY_HPP
#define TRAINTASTIC_SERVER_ROUTE_TRAINROUTEENTRY_HPP

#include "../core/object.hpp"
#include "../core/property.hpp"
#include "../core/objectproperty.hpp"
#include <traintastic/enum/trainrouteentrysource.hpp>

class BlockRailTile;
class TrainRoute;

class TrainRouteEntry : public Object
{
  CLASS_ID("train_route_entry")

public:
  ObjectProperty<BlockRailTile> block;
  Property<uint16_t> waitTimeMin;
  Property<uint16_t> waitTimeMax;
  Property<TrainRouteEntrySource> source;

  TrainRouteEntry(TrainRoute& route);
  TrainRouteEntry(TrainRoute& route, BlockRailTile& block_);

  std::string getObjectId() const final;

protected:
  void worldEvent(WorldState state, WorldEvent event) override;

private:
  TrainRoute& m_route;

  void updateEnabled();
};

#endif
