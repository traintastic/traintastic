/**
 * server/src/board/map/link.hpp
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

#ifndef TRAINTASTIC_SERVER_BOARD_MAP_LINK_HPP
#define TRAINTASTIC_SERVER_BOARD_MAP_LINK_HPP

#include <vector>
#include <array>
#include <memory>

class Tile;
class Node;
struct Connector;

class Link : public std::enable_shared_from_this<Link>
{
  private:
    struct Connection
    {
      Node* node = nullptr;
      size_t connectorIndex = 0;

      operator bool() const
      {
        return node;
      }
    };

    std::vector<std::shared_ptr<Tile>> m_tiles;
    std::array<Connection, 2> m_connections;

    Link(const Link&) = delete;
    Link& operator =(const Link&) = delete;

  public:
    Link();
    Link(std::vector<std::shared_ptr<Tile>> tiles);
#ifndef NDEBUG
    ~Link();
#endif

    const std::vector<std::shared_ptr<Tile>>& tiles() const { return m_tiles; }

    void connect(Node& node1, const Connector& connector1, Node& node2, const Connector& connector2);
    void disconnect();

    Node& getNext(Node& node);
};

#endif
