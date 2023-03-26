/**
 * server/src/vehicle/rail/poweredrailvehicle.hpp
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

#ifndef TRAINTASTIC_SERVER_VEHICLE_RAIL_POWEREDRAILVEHICLE_HPP
#define TRAINTASTIC_SERVER_VEHICLE_RAIL_POWEREDRAILVEHICLE_HPP

#include "railvehicle.hpp"
#include <traintastic/enum/direction.hpp>
#include "../../core/powerproperty.hpp"

class PoweredRailVehicle : public RailVehicle
{
  protected:
    PoweredRailVehicle(World& world, std::string_view id_);

    void worldEvent(WorldState state, WorldEvent event) override;

  public:
    PowerProperty power;

    void setDirection(Direction value);
    void setEmergencyStop(bool value);
    void setSpeed(double kmph);
};

#endif
