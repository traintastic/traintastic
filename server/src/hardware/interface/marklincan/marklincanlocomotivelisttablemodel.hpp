/**
 * server/src/hardware/interface/marklincan/marklincanlocomotivelisttablemodel.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_MARKLINCAN_MARKLINCANLOCOMOTIVELISTTABLEMODEL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_MARKLINCAN_MARKLINCANLOCOMOTIVELISTTABLEMODEL_HPP

#include "../../../core/tablemodel.hpp"

class MarklinCANLocomotiveList;

class MarklinCANLocomotiveListTableModel : public TableModel
{
  CLASS_ID("table_model.marklin_can_locomotive_list")

  friend class MarklinCANLocomotiveList;

  private:
    std::shared_ptr<MarklinCANLocomotiveList> m_list;

  public:
    MarklinCANLocomotiveListTableModel(MarklinCANLocomotiveList& list);
    ~MarklinCANLocomotiveListTableModel() override;

    std::string getText(uint32_t column, uint32_t row) const final;
};

#endif
