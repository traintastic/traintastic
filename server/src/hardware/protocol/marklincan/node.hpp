/**
 * server/src/hardware/protocol/marklincan/node.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLINCAN_NODE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLINCAN_NODE_HPP

#include <string>
#include <cstdint>
#include "message/statusdataconfig.hpp"

namespace MarklinCAN {

enum class DeviceId : uint16_t;

struct Node
{
  uint32_t uid = 0;
  uint8_t softwareVersionMajor = 0;
  uint8_t softwareVersionMinor = 0;
  DeviceId deviceId = static_cast<DeviceId>(0);
  uint32_t serialNumber = 0;
  std::string articleNumber;
  std::string deviceName;
  uint8_t numberOfReadings = 0;
  std::vector<StatusData::ReadingDescription> readings;
  uint8_t numberOfConfigurationChannels = 0;
  std::vector<StatusData::ConfigurationDescription> configurations;
};

}

#endif
