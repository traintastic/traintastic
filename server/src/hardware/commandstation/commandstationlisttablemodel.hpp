/**
 * server/src/hardware/commandstation/commandstationlisttablemodel.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_COMMANDSTATION_COMMANDSTATIONLISTTABLEMODEL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_COMMANDSTATION_COMMANDSTATIONLISTTABLEMODEL_HPP

#include "../../core/objectlisttablemodel.hpp"
#include "commandstationlist.hpp"

class CommandStationList;

class CommandStationListTableModel : public ObjectListTableModel<CommandStation>
{
  friend class CommandStationList;

  protected:
    void propertyChanged(BaseProperty& property, uint32_t row) final;

  public:
    CLASS_ID("command_station_list_table_model")

    static bool isListedProperty(const std::string& name);

    CommandStationListTableModel(CommandStationList& commandStationList);

    std::string getText(uint32_t column, uint32_t row) const final;
};

#endif
