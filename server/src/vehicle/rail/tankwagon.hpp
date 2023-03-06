/**
 * server/src/vehicle/rail/tankwagon.hpp
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

#ifndef TRAINTASTIC_SERVER_VEHICLE_RAIL_TANKWAGON_HPP
#define TRAINTASTIC_SERVER_VEHICLE_RAIL_TANKWAGON_HPP

#include "unpoweredrailvehicle.hpp"
#include "../../core/ratioproperty.hpp"
#include "../../core/volumeproperty.hpp"

class TankWagon : public UnpoweredRailVehicle
{
  private:
    void updateCargoLoaded();

  protected:
    void loaded() override;
    void worldEvent(WorldState state, WorldEvent event) final;

    double calcTotalWeight(WeightUnit unit) const override;

  public:
    CLASS_ID("vehicle.rail.tank_wagon")
    CREATE(TankWagon)

    static constexpr double cargoDensity = 1000; //!< in kg/m3 \todo change to property
    RatioProperty cargoLoaded;
    VolumeProperty cargoVolume;
    VolumeProperty cargoCapacity;

    TankWagon(World& world, std::string_view _id);
};

#endif
