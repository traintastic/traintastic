/**
 * server/src/hardware/interface/interfacelisttablemodel.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_INTERFACELISTTABLEMODEL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_INTERFACELISTTABLEMODEL_HPP

#include "../../core/objectlisttablemodel.hpp"
#include "interfacelist.hpp"

class InterfaceList;

class InterfaceListTableModel : public ObjectListTableModel<Interface>
{
  friend class InterfaceList;

  protected:
    void propertyChanged(BaseProperty& property, uint32_t row) final;

  public:
    CLASS_ID("table_model.list.interface")

    static constexpr uint32_t columnStatus = 2;

    static bool isListedProperty(std::string_view name);

    InterfaceListTableModel(InterfaceList& interfaceList);

    std::string getText(uint32_t column, uint32_t row) const final;
};

#endif
