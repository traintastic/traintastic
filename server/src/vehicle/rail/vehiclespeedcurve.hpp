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
#include "../../core/subobject.hpp"
#include "../../core/method.hpp"

class PoweredRailVehicle;

class VehicleSpeedCurve : public SubObject
{
  CLASS_ID("vehicle_speed_curve")
public:

  VehicleSpeedCurve(PoweredRailVehicle& _parent, std::string_view parentPropertyName);

  double getSpeedForStep(uint8_t step) const;

  uint8_t stepUpperBound(double speed) const;
  uint8_t stepLowerBound(double speed) const;

  Method<void(const std::string&)> importFromString;
  Method<std::string()> exportToString;

  inline bool isValid() const { return m_valid; }

protected:
  void load(WorldLoader& loader, const nlohmann::json& data) override;
  void save(WorldSaver& saver, nlohmann::json& data, nlohmann::json& state) const override;
  void loaded() override;

  nlohmann::json toJSON() const;
  bool fromJSON(const nlohmann::json& obj);

  void update();

private:
  std::array<double, 126> m_speedCurve;
  bool m_valid = false;
};

#endif // TRAINTASTIC_SERVER_VEHICLE_RAIL_VEHICLESPEEDCURVE_HPP
