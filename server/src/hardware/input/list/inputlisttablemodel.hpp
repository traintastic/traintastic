/**
 * server/src/hardware/input/list/inputlisttablemodel.hpp
 *
 * This file is part of the traintastic source code.
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INPUT_LIST_INPUTLISTTABLEMODEL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INPUT_LIST_INPUTLISTTABLEMODEL_HPP

#include "../../../core/objectlisttablemodel.hpp"
#include "../input.hpp"

class InputList;

class InputListTableModel : public ObjectListTableModel<Input>
{
  friend class InputList;

  private:
    static constexpr uint32_t columnId = 0;
    static constexpr uint32_t columnName = 1;
    const uint32_t m_columnInterface;
    const uint32_t m_columnAddress;

  protected:
    void propertyChanged(BaseProperty& property, uint32_t row) final;

  public:
    CLASS_ID("input_list_table_model")

    static bool isListedProperty(std::string_view name);

    InputListTableModel(InputList& list);

    std::string getText(uint32_t column, uint32_t row) const final;
};

#endif
