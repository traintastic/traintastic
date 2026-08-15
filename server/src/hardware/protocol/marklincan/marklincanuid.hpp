/**
 * server/src/hardware/protocol/marklincan/uid.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLINCAN_UID_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLINCAN_UID_HPP

#include <cstdint>
#include <string>
#include <utility>
#include <traintastic/enum/decoderprotocol.hpp>

namespace MarklinCAN::UID {

namespace Range
{
  constexpr std::pair<uint32_t, uint32_t> locomotiveMotorola{0x000, 0x03FF};
  constexpr std::pair<uint32_t, uint32_t> accessorySX1{0x0800, 0x0BFF};
  constexpr std::pair<uint32_t, uint32_t> manufacturer{0x1C00, 0x1FFF};
  constexpr std::pair<uint32_t, uint32_t> accessoryMotorola{0x3000, 0x33FF};
  constexpr std::pair<uint32_t, uint32_t> accessoryDCC{0x3800, 0x3FFF};
  constexpr std::pair<uint32_t, uint32_t> locomotiveMFX{0x4000, 0x7FFF};
  constexpr std::pair<uint32_t, uint32_t> locomotiveDCC{0xC000, 0xFFFF};
}

constexpr uint32_t locomotiveMotorola(uint16_t address)
{
  return (Range::locomotiveMotorola.first | address);
}

constexpr uint32_t accessorySX1(uint16_t address)
{
  return (Range::accessorySX1.first | (address - 1));
}

constexpr uint32_t accessoryMotorola(uint16_t address)
{
  return (Range::accessoryMotorola.first | (address - 1));
}

constexpr uint32_t accessoryDCC(uint16_t address)
{
  return (Range::accessoryDCC.first | (address - 1));
}

constexpr uint32_t locomotiveMFX(uint16_t address)
{
  return (Range::locomotiveMFX.first | address);
}

constexpr uint32_t locomotiveDCC(uint16_t address)
{
  return (Range::locomotiveDCC.first | address);
}

std::string toString(uint32_t uid);

}

#endif
