/**
 * server/src/board//map/node.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022,2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_BOARD_MAP_NODE_HPP
#define TRAINTASTIC_SERVER_BOARD_MAP_NODE_HPP

#include <memory>
#include <vector>
#include <limits>

class Tile;
class Link;
struct Connector;

class Node
{
  private:
    Tile& m_tile;
    std::vector<std::shared_ptr<Link>> m_links;

    Node(const Node&) = delete;
    Node& operator =(const Node&) = delete;

  public:
    static constexpr size_t noIndex = std::numeric_limits<size_t>::max();

    Node(Tile& tile, size_t connectors);
    ~Node();

    Tile& tile() const { return m_tile; }
    const std::vector<std::shared_ptr<Link>>& links() const { return m_links; }

    size_t getIndex(const Connector& connector) const;
    size_t getIndex(const Link& link) const;
    std::shared_ptr<const Link> getLink(size_t index) const;
    std::shared_ptr<Link> getLink(size_t index);

    bool connect(size_t index, Link& link);
    bool disconnect(size_t index, Link& link);
    void disconnect(const Connector& connector);
};

#endif
