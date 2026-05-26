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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_CBUS_CBUSNODELIST_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_CBUS_CBUSNODELIST_HPP

#include "../../../core/subobject.hpp"
#include "../../../core/table.hpp"
#include <optional>

class CBUSNodeListTableModel;

class CBUSNodeList : public SubObject, public Table
{
  CLASS_ID("cbus_node_list")

  friend class CBUSInterface;
  friend class CBUSNodeListTableModel;

public:
  struct Node
  {
    uint16_t nodeNumber;
    uint8_t canId;
    uint8_t manufacturerId;
    uint8_t moduleId;
    bool flim;
    bool vlcb;

    struct Parameters
    {
      std::optional<uint8_t> versionMajor;
      std::optional<uint8_t> versionMinor;
      std::optional<uint8_t> betaReleaseCode;
    } parameters;
  };

  CBUSNodeList(Object& parent_, std::string_view parentPropertyName);

  TableModelPtr getModel() final;

private:
  std::vector<Node> m_nodes;
  std::vector<CBUSNodeListTableModel*> m_models;

  void add(Node&& node);
  void clear();

  void rowChanged(uint32_t row);
};

#endif
