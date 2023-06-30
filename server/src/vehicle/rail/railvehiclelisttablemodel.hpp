/**
 * server/src/vehicle/rail/railvehiclelisttablemodel.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021,2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_VEHICLE_RAIL_RAILVEHICLELISTTABLEMODEL_HPP
#define TRAINTASTIC_SERVER_VEHICLE_RAIL_RAILVEHICLELISTTABLEMODEL_HPP

#include "../../core/objectlisttablemodel.hpp"
#include "railvehicle.hpp"

class RailVehicleList;

class RailVehicleListTableModel : public ObjectListTableModel<RailVehicle>
{
  friend class DecoderList;

  protected:
    void propertyChanged(BaseProperty& property, uint32_t row) final;

  public:
    CLASS_ID("rail_vehicle_list_table_model")

    static bool isListedProperty(std::string_view name);

    RailVehicleListTableModel(ObjectList<RailVehicle>& list);

    std::string getText(uint32_t column, uint32_t row) const final;
};

#endif
