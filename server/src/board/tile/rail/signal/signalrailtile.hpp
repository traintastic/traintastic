/**
 * server/src/board/tile/rail/signal/signalrailtile.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2021 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_BOARD_TILE_RAIL_SIGNAL_SIGNALRAILTILE_HPP
#define TRAINTASTIC_SERVER_BOARD_TILE_RAIL_SIGNAL_SIGNALRAILTILE_HPP

#include "../straightrailtile.hpp"
#include "../../../../core/method.hpp"
#include "../../../../enum/signalaspect.hpp"
#include "../../../../core/objectproperty.hpp"
#include "../../../../hardware/output/map/signaloutputmap.hpp"

class SignalRailTile : public StraightRailTile
{
  DEFAULT_ID("signal")

  protected:
    SignalRailTile(const std::weak_ptr<World>& world, std::string_view _id, TileId tileId);

    void worldEvent(WorldState state, WorldEvent event) override;

    virtual void doNextAspect(bool reverse) = 0;

  public:
    Property<std::string> name;
    Property<SignalAspect> aspect;
    ObjectProperty<SignalOutputMap> outputMap;
    Method<void(bool)> nextAspect;
};

#endif
