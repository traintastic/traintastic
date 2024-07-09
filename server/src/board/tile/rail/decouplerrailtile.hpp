/**
 * server/src/board/tile/rail/decouplerrailtile.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_BOARD_TILE_RAIL_DECOUPLERRAILTILE_HPP
#define TRAINTASTIC_SERVER_BOARD_TILE_RAIL_DECOUPLERRAILTILE_HPP

#include "straightrailtile.hpp"
#include "../../map/node.hpp"
#include "../../../core/method.hpp"
#include "../../../hardware/output/map/decoupleroutputmap.hpp"

class DecouplerRailTile final : public StraightRailTile
{
  CLASS_ID("board_tile.rail.decoupler")
  DEFAULT_ID("decoupler")
  CREATE_DEF(DecouplerRailTile)

  private:
    Node m_node;

    void setState(DecouplerState value, bool skipAction = false);

  protected:
    void destroying() override;
    void addToWorld() final;
    void worldEvent(WorldState worldState, WorldEvent worldEvent) final;

  public:
    Property<std::string> name;
    Property<DecouplerState> state;
    ObjectProperty<DecouplerOutputMap> outputMap;
    Method<void()> activate;
    Method<void()> deactivate;

    DecouplerRailTile(World& world, std::string_view _id);

    std::optional<std::reference_wrapper<const Node>> node() const final { return m_node; }
    std::optional<std::reference_wrapper<Node>> node() final { return m_node; }
};

#endif
