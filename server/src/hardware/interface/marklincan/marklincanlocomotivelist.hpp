/**
 * server/src/hardware/interface/marklincan/marklincanlocomotivelist.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_MARKLINCAN_MARKLINCANLOCOMOTIVELIST_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_MARKLINCAN_MARKLINCANLOCOMOTIVELIST_HPP

#include "../../../core/subobject.hpp"
#include "../../../core/table.hpp"
#include "../../../core/method.hpp"
#include "../../protocol/marklincan/locomotivelist.hpp"
#include <memory>

class MarklinCANInterface;
class MarklinCANLocomotiveListTableModel;

class MarklinCANLocomotiveList : public SubObject, public Table
{
  CLASS_ID("marklin_can_locomotive_list")

  friend class MarklinCANInterface;
  friend class MarklinCANLocomotiveListTableModel;

  private:
    std::shared_ptr<MarklinCAN::LocomotiveList> m_data;
    std::vector<MarklinCANLocomotiveListTableModel*> m_models;

    void worldEvent(WorldState state, WorldEvent event) override;

    void clear();
    MarklinCANInterface& interface();
    void import(const MarklinCAN::LocomotiveList::Locomotive& locomotive);
    void updateEnabled();

  public:
    Method<void(const std::string&)> importOrSync;
    Method<void()> importOrSyncAll;
    Method<void()> reload;

    MarklinCANLocomotiveList(Object& parent_, std::string_view parentPropertyName);

    const std::shared_ptr<MarklinCAN::LocomotiveList>& data() const
    {
      return m_data;
    }

    void setData(std::shared_ptr<MarklinCAN::LocomotiveList> value);

    TableModelPtr getModel() final;
};

#endif
