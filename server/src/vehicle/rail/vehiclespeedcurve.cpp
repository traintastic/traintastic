/**
 * server/src/vehicle/rail/vehiclespeedcurve.cpp
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

#include "vehiclespeedcurve.hpp"

#include <algorithm>

VehicleSpeedCurve::VehicleSpeedCurve(const std::array<double, 126> &arr)
  : mSpeedCurve(arr)
{

}

double VehicleSpeedCurve::getSpeedForStep(uint8_t step) const
{
  if(step <= 0 || step > 126)
    return 0;

  // We do not store zero so index is step - 1
  return mSpeedCurve.at(step - 1);
}

uint8_t VehicleSpeedCurve::stepUpperBound(double speed) const
{
  auto it = std::upper_bound(mSpeedCurve.begin(),
                             mSpeedCurve.end(),
                             speed);
  if(it != mSpeedCurve.end())
  {
    int idx = std::distance(mSpeedCurve.begin(), it);

    // We do not store zero so step is index + 1
    int step = idx + 1;
    return step;
  }
  return 0;
}

uint8_t VehicleSpeedCurve::stepLowerBound(double speed) const
{
  auto it = std::lower_bound(mSpeedCurve.begin(),
                             mSpeedCurve.end(),
                             speed);
  if(it != mSpeedCurve.end())
  {
    int idx = std::distance(mSpeedCurve.begin(), it);
    // We do not store zero so step is index + 1
    int step = idx + 1;
    return step;
  }
  return 0;
}
