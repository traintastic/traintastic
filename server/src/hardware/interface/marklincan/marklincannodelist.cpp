/**
 * server/src/hardware/interface/marklincan/marklincannodelist.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#include "marklincannodelist.hpp"
#include "marklincannodelisttablemodel.hpp"

MarklinCANNodeList::MarklinCANNodeList(Object& parent_, std::string_view parentPropertyName)
  : SubObject(parent_, parentPropertyName)
{
}

TableModelPtr MarklinCANNodeList::getModel()
{
  return std::make_shared<MarklinCANNodeListTableModel>(*this);
}

void MarklinCANNodeList::update(const MarklinCAN::Node& node)
{
  auto it = std::find_if(m_nodes.begin(), m_nodes.end(),
    [uid=node.uid](const auto& item)
    {
      return item.uid == uid;
    });

  if(it == m_nodes.end()) // add
  {
    m_nodes.push_back(node);
    const uint32_t rowCount = m_nodes.size();
    for(auto& model : m_models)
    {
      model->setRowCount(rowCount);
    }
  }
  else // update
  {
    *it = node;
    const uint32_t row = it - m_nodes.begin();
    for(auto& model : m_models)
    {
      model->rowsChanged(row, row);
    }
  }
}

void MarklinCANNodeList::clear()
{
  m_nodes.clear();
  for(auto& model : m_models)
  {
    model->setRowCount(0);
  }
}
