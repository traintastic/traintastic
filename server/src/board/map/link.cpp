/**
 * server/src/board/map/link.cpp
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

#include "link.hpp"
#include <cassert>
#include "node.hpp"

Link::Link() = default;

Link::Link(std::vector<std::shared_ptr<Tile>> tiles)
  : m_tiles{std::move(tiles)}
{
}

#ifndef NDEBUG
Link::~Link()
{
  assert(!m_connections[0]);
  assert(!m_connections[1]);
}
#endif

void Link::connect(Node& node1, const Connector& connector1, Node& node2, const Connector& connector2)
{
  // a link should only be connected once
  assert(!m_connections[0]);
  assert(!m_connections[1]);

  const size_t index1 = node1.getIndex(connector1);
  const size_t index2 = node2.getIndex(connector2);

  if(index1 == Node::noIndex || index2 == Node::noIndex)
    return;

  auto link1 = node1.getLink(index1);
  auto link2 = node2.getLink(index2);

  if(link1 && link2 && link1 == link2 &&
      (tiles() == link1->tiles() || std::equal(m_tiles.begin(), m_tiles.end(), link1->tiles().rbegin(), link1->tiles().rend())))
    return;

  if(node1.connect(index1, *this))
    m_connections[0] = {&node1, index1};
  else
    return;

  if(node2.connect(index2, *this))
    m_connections[1] = {&node2, index2};
  else
    disconnect();
}

void Link::disconnect()
{
  auto keepAlive = shared_from_this();
  for(auto& connection : m_connections)
    if(connection)
    {
      if(connection.node->disconnect(connection.connectorIndex, *this))
        connection.node = nullptr;
      else
        assert(false);
    }
}

const Node& Link::getNext(const Node& node) const
{
  assert(m_connections[0].node == &node || m_connections[1].node == &node);
  return *m_connections[(m_connections[0].node == &node) ? 1 : 0].node;
}

Node& Link::getNext(Node& node)
{
  assert(m_connections[0].node == &node || m_connections[1].node == &node);
  return *m_connections[(m_connections[0].node == &node) ? 1 : 0].node;
}
