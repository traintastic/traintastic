/**
 * server/src/hardware/protocol/traintasticcs/config.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_TRAINTASTICCS_CONFIG_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_TRAINTASTICCS_CONFIG_HPP

#include <chrono>

namespace TraintasticCS {

struct Config
{
  static constexpr auto responseTimeout = std::chrono::milliseconds(100);
  static constexpr auto resetResponseTimeout = std::chrono::milliseconds(1000);
  static constexpr auto pingTimeout = std::chrono::milliseconds(1000);

  struct LocoNet
  {
    bool enabled;
  } loconet;

  struct S88
  {
    static constexpr uint8_t moduleCountMin = 1;
    static constexpr uint8_t moduleCountMax = 16;
    static constexpr uint8_t clockFrequencyMin = 1; // kHz
    static constexpr uint8_t clockFrequencyMax = 250; // kHz

    bool enabled;
    uint8_t moduleCount;
    uint8_t clockFrequency;
  } s88;

  struct XpressNet
  {
    bool enabled;
  } xpressnet;

  bool debugLogRXTX;
  bool debugLogPing;
};

}

#endif
