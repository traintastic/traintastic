/**
 * server/src/hardware/interface/marklincan/marklincannodelist.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_MARKLINCAN_MARKLINCANNODELIST_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_MARKLINCAN_MARKLINCANNODELIST_HPP

#include "../../../core/subobject.hpp"
#include "../../../core/table.hpp"
#include "../../protocol/marklincan/node.hpp"

class MarklinCANInterface;
class MarklinCANNodeListTableModel;

class MarklinCANNodeList : public SubObject, public Table
{
  CLASS_ID("marklin_can_node_list")

  friend class MarklinCANInterface;
  friend class MarklinCANNodeListTableModel;

  private:
    std::vector<MarklinCAN::Node> m_nodes;
    std::vector<MarklinCANNodeListTableModel*> m_models;

    void update(const MarklinCAN::Node& node);
    void clear();

  public:
    MarklinCANNodeList(Object& parent_, std::string_view parentPropertyName);

    TableModelPtr getModel() final;
};

#endif
