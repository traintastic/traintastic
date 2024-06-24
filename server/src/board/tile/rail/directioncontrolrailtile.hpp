/**
 * server/src/board/tile/rail/directioncontrolrailtile.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_BOARD_TILE_RAIL_DIRECTIONCONTROLRAILTILE_HPP
#define TRAINTASTIC_SERVER_BOARD_TILE_RAIL_DIRECTIONCONTROLRAILTILE_HPP

#include "straightrailtile.hpp"
#include "../../map/node.hpp"
#include "../../../core/method.hpp"
#include "../../../enum/directioncontrolstate.hpp"

class DirectionControlRailTile final : public StraightRailTile
{
  CLASS_ID("board_tile.rail.direction_control")
  DEFAULT_ID("direction")
  CREATE_DEF(DirectionControlRailTile)

  private:
    Node m_node;
    DirectionControlState m_reservedState;

    void updateEnabled();
    void updateStateValues();

  protected:
    void loaded() final;
    void worldEvent(WorldState worldState, WorldEvent worldEvent) final;

  public:
    boost::signals2::signal<void (const DirectionControlRailTile&, DirectionControlState)> stateChanged;

    Property<std::string> name;
    Property<bool> useNone;
    Property<bool> useAtoB;
    Property<bool> useBtoA;
    Property<bool> useBoth;
    Property<DirectionControlState> state;
    Method<bool(DirectionControlState)> setState;
    Method<bool()> toggle;

    DirectionControlRailTile(World& world, std::string_view _id);

    std::optional<std::reference_wrapper<const Node>> node() const final { return m_node; }
    std::optional<std::reference_wrapper<Node>> node() final { return m_node; }

    bool reserve(DirectionControlState turnoutPosition, bool dryRun = false);
};

#endif
