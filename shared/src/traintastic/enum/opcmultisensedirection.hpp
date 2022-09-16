/**
 * shared/src/traintastic/enum/opcmultisensedirection.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_OPCMULTISENSEDIRECTION_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_OPCMULTISENSEDIRECTION_HPP

#include <cstdint>
#include <array>
#include "enum.hpp"

enum class OPCMultiSenseDirection : uint8_t
{
  None = 0,
  InSensorAddress = 1,
  InTransponderAddress = 2,
};

TRAINTASTIC_ENUM(OPCMultiSenseDirection, "opc_multi_sense_direction", 3,
{
  {OPCMultiSenseDirection::None, "none"},
  {OPCMultiSenseDirection::InSensorAddress, "in_sensor_address"},
  {OPCMultiSenseDirection::InTransponderAddress, "in_transponder_address"},
});

inline constexpr std::array<OPCMultiSenseDirection, 3> opcMultiSenseDirectionValues{{
  OPCMultiSenseDirection::None,
  OPCMultiSenseDirection::InSensorAddress,
  OPCMultiSenseDirection::InTransponderAddress,
}};

#endif
