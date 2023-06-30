/**
 * server/src/hardware/protocol/loconet/config.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_CONFIG_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_CONFIG_HPP

#include <traintastic/enum/loconetf9f28.hpp>
#include <traintastic/enum/loconetfastclock.hpp>
#include <traintastic/enum/pcapoutput.hpp>

namespace LocoNet {

struct Config
{
  static constexpr uint16_t timeoutMin = 100; //!< Minimum timeout in milliseconds
  static constexpr uint16_t timeoutMax = 10000; //!< Maximum timeout in milliseconds

  uint16_t echoTimeout; //!< Wait for echo timeout in milliseconds
  uint16_t responseTimeout; //!< Wait for response timeout in milliseconds

  uint8_t locomotiveSlots; //!< Number of available locomotive slots, defaults to #SLOT_LOCO_MAX
  LocoNetF9F28 f9f28;

  LocoNetFastClock fastClock;
  bool fastClockSyncEnabled;
  uint8_t fastClockSyncInterval; //!< Fast clock sync interval in seconds

  bool debugLogInput;
  bool debugLogOutput;
  bool debugLogRXTX;

  bool pcap;
  PCAPOutput pcapOutput;
  bool listenOnly; //!< If enabled Traintastic will not send any message to the LocoNet, just for using Traintastic as LocoNet monitor.
};

}

#endif
