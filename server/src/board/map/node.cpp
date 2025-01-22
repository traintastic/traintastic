/**
 * server/src/board/map/node.cpp
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

#include "node.hpp"
#include "link.hpp"
#include "../tile/tile.hpp"

Node::Node(Tile& tile, size_t connectors)
  : m_tile{tile}
  , m_links{connectors}
{
}

Node::~Node()
{
  for(auto& link : m_links)
    if(link)
      link->disconnect();
}

size_t Node::getIndex(const Connector& connector) const
{
  std::vector<Connector> connectors;
  connectors.reserve(m_links.size());
  m_tile.getConnectors(connectors);
  assert(connectors.size() == m_links.size());

  for(size_t i = 0; i < connectors.size(); i++)
    if(connectors[i] == connector)
      return i;

  return noIndex;
}

size_t Node::getIndex(const Link& link) const
{
  const size_t size = m_links.size();
  for(size_t i = 0; i < size; ++i)
  {
    if(m_links[i].get() == &link)
    {
      return i;
    }
  }
  return std::numeric_limits<size_t>::max();
}

std::shared_ptr<const Link> Node::getLink(size_t index) const
{
  assert(index < m_links.size());
  return m_links[index];
}

std::shared_ptr<Link> Node::getLink(size_t index)
{
  assert(index < m_links.size());
  return m_links[index];
}

bool Node::connect(size_t index, Link& link)
{
  if(m_links[index])
    m_links[index]->disconnect();
  assert(!m_links[index]);
  m_links[index] = link.shared_from_this();
  return true;
}

bool Node::disconnect(size_t index, Link& link)
{
  assert(index < m_links.size());
  if(m_links[index].get() == &link)
  {
    m_links[index].reset();
    return true;
  }
  return false;
}

void Node::disconnect(const Connector& connector)
{
  if(auto index = getIndex(connector); index != noIndex && m_links[index])
    m_links[index]->disconnect();
}
