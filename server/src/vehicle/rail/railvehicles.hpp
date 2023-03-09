/**
 * server/src/vehicle/rail/railvehicles.hpp
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

#ifndef TRAINTASTIC_SERVER_VEHICLE_RAIL_RAILVEHICLES_HPP
#define TRAINTASTIC_SERVER_VEHICLE_RAIL_RAILVEHICLES_HPP

#include "railvehicle.hpp"
#include "../../utils/makearray.hpp"

#include "locomotive.hpp"
#include "multipleunit.hpp"
#include "freightwagon.hpp"
#include "tankwagon.hpp"

struct RailVehicles
{
  static constexpr std::string_view classIdPrefix = "vehicle.rail.";

  static constexpr auto classList = makeArray(
    Locomotive::classId,
    MultipleUnit::classId,
    FreightWagon::classId,
    TankWagon::classId
  );

  static std::shared_ptr<RailVehicle> create(World& world, std::string_view classId, std::string_view id);
};

#endif