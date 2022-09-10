/**
 * server/src/board/tile/rail/linkrailtile.hpp
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

#ifndef TRAINTASTIC_SERVER_BOARD_TILE_RAIL_LINKRAILTILE_HPP
#define TRAINTASTIC_SERVER_BOARD_TILE_RAIL_LINKRAILTILE_HPP

#include "railtile.hpp"
#include "../../map/node.hpp"
#include "../../../core/objectproperty.hpp"

class LinkRailTile : public RailTile
{
  CLASS_ID("board_tile.rail.link")
  CREATE(LinkRailTile)
  DEFAULT_ID("link")

  private:
    Node m_node;

  protected:
    void addToWorld() final;
    void loaded() final;
    void destroying() final;
    void worldEvent(WorldState worldState, WorldEvent worldEvent) final;

  public:
    Property<std::string> name;
    ObjectProperty<LinkRailTile> link;

    LinkRailTile(World& world, std::string_view _id);

    std::optional<std::reference_wrapper<const Node>> node() const final { return m_node; }
    std::optional<std::reference_wrapper<Node>> node() final { return m_node; }

    void getConnectors(std::vector<Connector>& connectors) const final;
};

#endif