/**
 * server/src/hardware/protocol/selectrix/utils.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023,2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_SELECTRIX_UTILS_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_SELECTRIX_UTILS_HPP

#include "bus.hpp"
#include <traintastic/enum/outputchannel.hpp>

namespace Selectrix {

constexpr Bus toBus(uint32_t channel)
{
  return static_cast<Bus>(channel - 1);
}

constexpr uint32_t toChannel(Bus bus)
{
  return 1 + static_cast<uint8_t>(bus);
}

constexpr uint8_t toBusAddress(uint32_t flatAddress)
{
  return (flatAddress - 1) / 8;
}

constexpr uint8_t toPort(uint32_t flatAddress)
{
  return (flatAddress - 1) % 8;
}

constexpr uint32_t toFlatAddress(uint8_t busAddress, uint8_t port)
{
  return 1 + static_cast<uint32_t>(busAddress * 8) + port;
}

namespace Accessory {

constexpr Bus toBus(OutputChannel channel)
{
  switch(channel)
  {
    case OutputChannel::AccessorySX0:
      return Bus::SX0;

    case OutputChannel::AccessorySX1:
      return Bus::SX1;

    case OutputChannel::AccessorySX2:
      return Bus::SX2;

    default:
      break;
  }
  return static_cast<Bus>(-1);
}

constexpr OutputChannel toChannel(Bus bus)
{
  switch(bus)
  {
    case Bus::SX0:
      return OutputChannel::AccessorySX0;

    case Bus::SX1:
      return OutputChannel::AccessorySX1;

    case Bus::SX2:
      return OutputChannel::AccessorySX2;

    default:
      break;
  }
  return static_cast<OutputChannel>(-1);
}

constexpr uint8_t toBusAddress(uint32_t flatAddress)
{
  return (flatAddress - 1) / 4;
}

constexpr uint8_t toPort(uint32_t flatAddress)
{
  return (flatAddress - 1) % 4;
}

}

}

#endif
