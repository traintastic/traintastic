/**
 * server/src/vehicle/rail/freightwagon.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_VEHICLE_RAIL_FREIGHTWAGON_HPP
#define TRAINTASTIC_SERVER_VEHICLE_RAIL_FREIGHTWAGON_HPP

#include "unpoweredrailvehicle.hpp"
#include "../../core/ratioproperty.hpp"

class FreightWagon : public UnpoweredRailVehicle
{
  private:
    void updateCargoLoaded();

  protected:
    void loaded() override;
    void worldEvent(WorldState state, WorldEvent event) final;

    double calcTotalWeight(WeightUnit unit) const override;

  public:
    CLASS_ID("vehicle.rail.freight_wagon")
    CREATE(FreightWagon)

    RatioProperty cargoLoaded;
    WeightProperty cargoWeight;
    WeightProperty cargoCapacity;

    FreightWagon(World& world, std::string_view _id);
};

#endif
