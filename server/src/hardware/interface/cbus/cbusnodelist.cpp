/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#include "cbusnodelist.hpp"
#include "cbusnodelisttablemodel.hpp"

CBUSNodeList::CBUSNodeList(Object& parent_, std::string_view parentPropertyName)
  : SubObject(parent_, parentPropertyName)
{
}

TableModelPtr CBUSNodeList::getModel()
{
  return std::make_shared<CBUSNodeListTableModel>(*this);
}

void CBUSNodeList::add(Node&& node)
{
  m_nodes.emplace_back(std::move(node));

  const auto rowCount = static_cast<uint32_t>(m_nodes.size());
  for(auto& model : m_models)
  {
    model->setRowCount(rowCount);
  }
}

void CBUSNodeList::clear()
{
  m_nodes.clear();
  for(auto& model : m_models)
  {
    model->setRowCount(0);
  }
}

void CBUSNodeList::rowChanged(uint32_t row)
{
  for(auto& model : m_models)
  {
    model->rowsChanged(row, row);
  }
}
