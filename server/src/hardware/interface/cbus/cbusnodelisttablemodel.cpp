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

#include "cbusnodelisttablemodel.hpp"
#include "cbusnodelist.hpp"
#include "../../../compat/stdformat.hpp"
#include "../../protocol/cbus/cbusmanufacturermodule.hpp"

constexpr uint32_t columnCANID = 0;
constexpr uint32_t columnNodeNumber = 1;
constexpr uint32_t columnManufacturer = 2;
constexpr uint32_t columnModule = 3;
constexpr uint32_t columnVersion = 4;
constexpr uint32_t columnMode = 5;
constexpr uint32_t columnType = 6;

CBUSNodeListTableModel::CBUSNodeListTableModel(CBUSNodeList& list)
  : m_list{list.shared_ptr<CBUSNodeList>()}
{
  assert(m_list);
  m_list->m_models.push_back(this);

  setColumnHeaders({
    std::string_view{"cbus_node_list:can_id"},
    std::string_view{"cbus_node_list:node_number"},
    std::string_view{"cbus_node_list:manufacturer"},
    std::string_view{"cbus_node_list:module"},
    std::string_view{"cbus_node_list:version"},
    std::string_view{"cbus_node_list:mode"},
    std::string_view{"cbus_node_list:type"}
  });

  setRowCount(m_list->m_nodes.size());
}

CBUSNodeListTableModel::~CBUSNodeListTableModel()
{
  auto it = std::find(m_list->m_models.begin(), m_list->m_models.end(), this);
  assert(it != m_list->m_models.end());
  m_list->m_models.erase(it);
}

std::string CBUSNodeListTableModel::getText(uint32_t column, uint32_t row) const
{
  if(row < m_list->m_nodes.size())
  {
    const auto& node = m_list->m_nodes[row];

    switch(column)
    {
      case columnCANID:
        return std::to_string(node.canId);

      case columnNodeNumber:
        return std::to_string(node.nodeNumber);

      case columnManufacturer:
        if(auto sv = CBUS::manufacturerName(node.manufacturerId); !sv.empty())
        {
          return std::string(sv);
        }
        return std::to_string(node.manufacturerId);

      case columnModule:
        if(auto sv = CBUS::moduleName(node.manufacturerId, node.moduleId); !sv.empty())
        {
          return std::string(sv);
        }
        return std::to_string(node.moduleId);

      case columnVersion:
        if(node.parameters.versionMajor && node.parameters.versionMinor && node.parameters.betaReleaseCode)
        {
          auto s = std::to_string(*node.parameters.versionMajor);
          s += static_cast<char>(*node.parameters.versionMinor);
          if(*node.parameters.betaReleaseCode > 0)
          {
            s += std::format(" Beta {}", *node.parameters.betaReleaseCode);
          }
          return s;
        }
        break;

      case columnMode:
        return node.flim ? "FLiM" : "SLiM";

      case columnType:
        return node.vlcb ? "VLCB" : "CBUS";
    }
  }
  return {};
}
