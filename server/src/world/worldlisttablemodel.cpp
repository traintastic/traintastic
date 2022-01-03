/**
 * server/src/world/worldlisttablemodel.cpp
 *
 * This file is part of the traintastic source code
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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

#include "worldlisttablemodel.hpp"
#include "worldlist.hpp"
#include <boost/uuid/uuid_io.hpp>
#include "../utils/displayname.hpp"

constexpr uint32_t columnName = 0;
constexpr uint32_t columnUUID = 1;

WorldListTableModel::WorldListTableModel(WorldList& worldList) :
  m_worldList{worldList.shared_ptr<WorldList>()}
{
  m_worldList->m_models.push_back(this);
  setRowCount(static_cast<uint32_t>(m_worldList->m_items.size()));

  setColumnHeaders({
    DisplayName::Object::name,
    DisplayName::World::uuid,
    });
}

WorldListTableModel::~WorldListTableModel()
{
  auto it = std::find(m_worldList->m_models.begin(), m_worldList->m_models.end(), this);
  assert(it != m_worldList->m_models.end());
  m_worldList->m_models.erase(it);
}

std::string WorldListTableModel::getText(uint32_t column, uint32_t row) const
{
  if(row < rowCount())
  {
    auto& item = m_worldList->m_items[row];

    switch(column)
    {
      case columnName:
        return item.name;

      case columnUUID:
        return to_string(item.uuid);

      default:
        assert(false);
        break;
    }
  }

  return "";
}

