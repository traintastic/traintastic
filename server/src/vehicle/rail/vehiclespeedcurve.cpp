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
#include "../../core/to.hpp"

#include <algorithm>

VehicleSpeedCurve::VehicleSpeedCurve(const std::array<double, 126> &arr)
  : m_speedCurve(arr)
{

}

double VehicleSpeedCurve::getSpeedForStep(uint8_t step) const
{
  if(step <= 0 || step > 126)
    return 0;

  // We do not store zero so index is step - 1
  return m_speedCurve.at(step - 1);
}

uint8_t VehicleSpeedCurve::stepUpperBound(double speed) const
{
  auto it = std::upper_bound(m_speedCurve.begin(),
                             m_speedCurve.end(),
                             speed);
  if(it != m_speedCurve.end())
  {
    int idx = std::distance(m_speedCurve.begin(), it);

    // We do not store zero so step is index + 1
    int step = idx + 1;
    return step;
  }
  return 0;
}

uint8_t VehicleSpeedCurve::stepLowerBound(double speed) const
{
  auto it = std::lower_bound(m_speedCurve.begin(),
                             m_speedCurve.end(),
                             speed);
  if(it != m_speedCurve.end())
  {
    int idx = std::distance(m_speedCurve.begin(), it);
    // We do not store zero so step is index + 1
    int step = idx + 1;
    return step;
  }
  return 0;
}

nlohmann::json VehicleSpeedCurve::toJSON() const
{
    nlohmann::json values(nlohmann::json::value_t::array);
    for(double speed : m_speedCurve)
      values.push_back(speed);

    nlohmann::json obj;
    obj["values"] = values;

    return obj;
}

void VehicleSpeedCurve::fromJSON(const nlohmann::json& obj)
{
  nlohmann::json values = obj["values"];
  if(!values.is_array() || values.size() != 126)
    throw conversion_error();

  for(size_t i = 0; i < values.size(); i++)
    m_speedCurve[i] = to<double>(values[i]);
}

bool VehicleSpeedCurve::loadFromString(const std::string &str)
{
    nlohmann::json doc = nlohmann::json::parse(str);
    try
    {
      fromJSON(doc);
    }
    catch(...)
    {
      return false;
    }
    return true;
}
