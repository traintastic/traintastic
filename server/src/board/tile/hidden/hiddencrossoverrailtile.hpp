/**
 * server/src/board/tile/hidden/hiddencrossoverrailtile.hpp
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

#ifndef TRAINTASTIC_SERVER_BOARD_TILE_HIDDEN_HIDDENCROSSOVERRAILTILE_HPP
#define TRAINTASTIC_SERVER_BOARD_TILE_HIDDEN_HIDDENCROSSOVERRAILTILE_HPP

#include "hiddentile.hpp"
#include "../../map/node.hpp"

enum class CrossState : uint8_t;

class HiddenCrossOverRailTile : public HiddenTile
{
  private:
    Node m_node;
    CrossState m_crossState; //!< indicates which path is reserved

  public:
    HiddenCrossOverRailTile(World& world);

    std::string_view getClassId() const final;

    std::optional<std::reference_wrapper<const Node>> node() const final { return m_node; }
    std::optional<std::reference_wrapper<Node>> node() final { return m_node; }
    void getConnectors(std::vector<Connector>& connectors) const final;

    bool reserve(CrossState crossState, bool dryRun = false);
    bool release(bool dryRun = false);
};

#endif
