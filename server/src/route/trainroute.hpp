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

#ifndef TRAINTASTIC_SERVER_ROUTE_TRAINROUTE_HPP
#define TRAINTASTIC_SERVER_ROUTE_TRAINROUTE_HPP

#include "../core/idobject.hpp"
#include "../core/method.hpp"
#include "../core/objectvectorproperty.hpp"

class BlockRailTile;
class TrainRouteEntry;

class TrainRoute : public IdObject
{
  friend class TrainRouteList;

  CLASS_ID("train_route")
  CREATE_DEF(TrainRoute)

public:
  Property<std::string> name;
  Property<bool> enabled;
  Property<bool> temporary;
  Property<bool> valid;
  ObjectVectorProperty<TrainRouteEntry> entries;
  ObjectVectorProperty<TrainRouteEntry> entriesResolved;
  Method<void(const std::shared_ptr<BlockRailTile>&)> addBlock;
  Method<void(const std::shared_ptr<TrainRouteEntry>&)> remove;

  TrainRoute(World& world, std::string_view _id);

protected:
  void loaded() override;
  void addToWorld() override;
  void destroying() override;
  void worldEvent(WorldState state, WorldEvent event) override;

private:
  void resolve();
  void updateEnabled();
};

#endif
