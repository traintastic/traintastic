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
#include "poweredrailvehicle.hpp"
#include "../../core/attributes.hpp"
#include "../../core/method.tpp"
#include "../../core/to.hpp"

#include <algorithm>

VehicleSpeedCurve::VehicleSpeedCurve(PoweredRailVehicle& _parent, std::string_view parentPropertyName)
  : SubObject(_parent, parentPropertyName)
  , importFromString{*this, "import_from_string", MethodFlags::ScriptCallable,
    [this](const std::string& str)
    {
      bool success = fromJSON(nlohmann::json::parse(str));
      m_valid = success;
      if(!success)
        m_speedCurve = {};

      update();

      if(!success)
        throw std::runtime_error("File format is invalid");
    }}
  , exportToString{*this, "export_to_string", MethodFlags::ScriptCallable,
    [this]() -> std::string
    {
      nlohmann::json obj = toJSON();
      return obj.dump();
    }}
{
  m_interfaceItems.add(importFromString);

  Attributes::addVisible(exportToString, false);
  m_interfaceItems.add(exportToString);
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

void VehicleSpeedCurve::load(WorldLoader &loader, const nlohmann::json &data)
{
  m_valid = false;

  SubObject::load(loader, data);

  nlohmann::json values = data.value("values", nlohmann::json());
  if(!values.is_array() || values.size() != 126)
    return;

  for(size_t i = 0; i < values.size(); i++)
    m_speedCurve[i] = to<double>(values[i]);

  m_valid = true;
}

void VehicleSpeedCurve::save(WorldSaver &saver, nlohmann::json &data, nlohmann::json &state) const
{
  SubObject::save(saver, data, state);

  if(!m_valid)
    return;

  nlohmann::json values(nlohmann::json::value_t::array);
  for(double speed : m_speedCurve)
    values.push_back(speed);

  data["values"] = values;
}

void VehicleSpeedCurve::loaded()
{
  Attributes::setVisible(exportToString, m_valid);
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

bool VehicleSpeedCurve::fromJSON(const nlohmann::json& obj)
{
  nlohmann::json values = obj.value("curve_array", nlohmann::json());
  if(values.is_array() && values.size() == 126)
  {
    for(size_t i = 0; i < values.size(); i++)
      m_speedCurve[i] = to<double>(values[i]);
    return true;
  }

  values = obj.value("speed_mapping", nlohmann::json());
  if(values.is_array())
  {
    double lastSpeed = 0;
    uint8_t lastStep = 0;

    for(const nlohmann::json& item : values)
    {
      uint8_t step = item.value<uint8_t>("step", 0);
      if(step == 0)
        continue;

      double speed = item.value<double>("speed", 0.0);

      if(step > lastStep + 1)
      {
        // Linear interpolation of steps inbetween
        uint8_t numSteps = step - lastStep;
        double increment = (speed - lastSpeed) / double(numSteps);

        for(int i = 1; i < numSteps; i++)
        {
          double calculatedSpeed = lastSpeed + increment * double(i);
          uint8_t calculatedStep = lastStep + i;

          if(step > 0 && step <= 126)
            m_speedCurve[calculatedStep - 1] = calculatedSpeed;
        }
      }

      lastStep = step;
      lastSpeed = speed;

      if(step > 0 && step <= 126)
        m_speedCurve[step - 1] = speed;
    }

    return true;
  }

  return false;
}

void VehicleSpeedCurve::update()
{
  Attributes::setVisible(exportToString, m_valid);
  static_cast<PoweredRailVehicle&>(parent()).updateMaxSpeed();
}
