/**
 * server/src/vehicle/rail/vehiclespeedcurve.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Filippo Gentile
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

#ifndef TRAINTASTIC_SERVER_VEHICLE_RAIL_VEHICLESPEEDCURVE_HPP
#define TRAINTASTIC_SERVER_VEHICLE_RAIL_VEHICLESPEEDCURVE_HPP

#include <array>
#include <cstdint>

class VehicleSpeedCurve
{
public:
  VehicleSpeedCurve(const std::array<double, 126>& arr = {});

  double getSpeedForStep(uint8_t step) const;

  uint8_t stepUpperBound(double speed) const;
  uint8_t stepLowerBound(double speed) const;

private:
  std::array<double, 126> mSpeedCurve;
};

#endif // TRAINTASTIC_SERVER_VEHICLE_RAIL_VEHICLESPEEDCURVE_HPP
