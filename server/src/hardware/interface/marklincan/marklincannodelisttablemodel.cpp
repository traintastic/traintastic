/**
 * server/src/hardware/interface/marklincan/marklincannodelisttablemodel.cpp
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

#include "marklincannodelisttablemodel.hpp"
#include "marklincannodelist.hpp"
#include "../../../utils/displayname.hpp"
#include "../../../utils/tohex.hpp"

constexpr uint32_t columnUID = 0;
constexpr uint32_t columnName = 1;
constexpr uint32_t columnArticleNumber = 2;
constexpr uint32_t columnSerialNumber = 3;
constexpr uint32_t columnSoftwareVersion = 4;
constexpr uint32_t columnDeviceId = 5;

MarklinCANNodeListTableModel::MarklinCANNodeListTableModel(MarklinCANNodeList& list)
  : m_list{list.shared_ptr<MarklinCANNodeList>()}
{
  assert(m_list);
  m_list->m_models.push_back(this);

  setColumnHeaders({
    std::string_view{"table_model.marklin_can_node_list:uid"},
    std::string_view{"table_model.marklin_can_node_list:device_name"},
    std::string_view{"table_model.marklin_can_node_list:article_number"},
    std::string_view{"table_model.marklin_can_node_list:serial_number"},
    std::string_view{"table_model.marklin_can_node_list:software_version"},
    std::string_view{"table_model.marklin_can_node_list:device_id"}
    });

  setRowCount(m_list->m_nodes.size());
}

MarklinCANNodeListTableModel::~MarklinCANNodeListTableModel()
{
  auto it = std::find(m_list->m_models.begin(), m_list->m_models.end(), this);
  assert(it != m_list->m_models.end());
  m_list->m_models.erase(it);
}

std::string MarklinCANNodeListTableModel::getText(uint32_t column, uint32_t row) const
{
  if(row < m_list->m_nodes.size())
  {
    const auto& node = m_list->m_nodes[row];

    switch(column)
    {
      case columnUID:
        return toHex(node.uid);

      case columnName:
        return node.deviceName;

      case columnArticleNumber:
        return node.articleNumber;

      case columnSerialNumber:
        if(node.serialNumber == 0)
          return {};
        return std::to_string(node.serialNumber);

      case columnSoftwareVersion:
        return std::to_string(node.softwareVersionMajor).append(".").append(std::to_string(node.softwareVersionMinor));

      case columnDeviceId:
        return toHex(static_cast<uint16_t>(node.deviceId));
    }
  }
  return {};
}
