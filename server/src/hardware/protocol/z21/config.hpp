/**
 * server/src/hardware/protocol/z21/config.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_Z21_CONFIG_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_Z21_CONFIG_HPP

#include "messages.hpp"

namespace Z21 {

struct Config
{
  bool debugLogRXTX;
};

struct ClientConfig : Config
{
  static constexpr uint16_t keepAliveInterval = 15; //!< sec
  static constexpr uint16_t purgeInactiveDecoderInternal = 5 * 60; //!< sec
};

struct ServerConfig : Config
{
  static constexpr CommandStationId commandStationId = CommandStationId::Z21;
  static constexpr uint8_t firmwareVersionMajor = 1;
  static constexpr uint8_t firmwareVersionMinor = 30;
  static constexpr HardwareType hardwareType = HWT_Z21_START;
  static constexpr uint16_t inactiveClientPurgeTime = 60; ///< sec
  static constexpr uint32_t serialNumber = 123456789;
  static constexpr size_t subscriptionMax = 16;
  static constexpr uint8_t xBusVersion = 30;

  bool allowEmergencyStop;
  bool allowTrackPowerOff;
  bool allowTrackPowerOnReleaseEmergencyStop;
};

}

#endif
