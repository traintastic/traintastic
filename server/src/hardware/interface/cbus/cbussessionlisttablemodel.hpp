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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_CBUS_CBUSSESSIONLISTTABLEMODEL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_CBUS_CBUSSESSIONLISTTABLEMODEL_HPP

#include "../../../core/tablemodel.hpp"

class CBUSSessionList;

class CBUSSessionListTableModel : public TableModel
{
  CLASS_ID("table_model.cbus_session_list")

  friend class CBUSSessionList;

public:
  CBUSSessionListTableModel(CBUSSessionList& list);
  ~CBUSSessionListTableModel() override;

  std::string getText(uint32_t column, uint32_t row) const final;

private:
  std::shared_ptr<CBUSSessionList> m_list;
};

#endif
