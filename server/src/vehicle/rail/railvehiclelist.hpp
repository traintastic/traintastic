/**
 * server/src/vehicle/rail/railvehicle.hpp
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

#ifndef TRAINTASTIC_SERVER_VEHICLE_RAIL_RAILVEHICLELIST_HPP
#define TRAINTASTIC_SERVER_VEHICLE_RAIL_RAILVEHICLELIST_HPP

#include "../../core/objectlist.hpp"
#include "railvehicle.hpp"

class RailVehicleList : public ObjectList<RailVehicle>
{
  protected:
    void worldEvent(WorldState state, WorldEvent event) final;
    bool isListedProperty(std::string_view name) final;

  public:
    CLASS_ID("rail_vehicle_list")

    Method<std::shared_ptr<RailVehicle>(std::string_view)> create;
    Method<void(const std::shared_ptr<RailVehicle>&)> delete_;

    RailVehicleList(Object& _parent, std::string_view parentPropertyName);

    TableModelPtr getModel() final;
};

#endif
