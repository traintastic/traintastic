/**
 * server/src/hardware/protocol/dcc/dcc.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DCC_DCC_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DCC_DCC_HPP

#include <cstdint>
#include <traintastic/enum/decoderprotocol.hpp>
#include "../../../utils/inrange.hpp"

namespace DCC {

constexpr uint16_t addressBroadcast = 0;
constexpr uint16_t addressMin = 1;
constexpr uint16_t addressShortMax = 127;
constexpr uint16_t addressLongStart = 128;
constexpr uint16_t addressLongMax = 10239;

constexpr bool isLongAddress(uint16_t address)
{
  return inRange(address, addressLongStart, addressLongMax);
}

constexpr DecoderProtocol getProtocol(uint16_t address)
{
  return isLongAddress(address) ? DecoderProtocol::DCCLong : DecoderProtocol::DCCShort;
}

}

#endif
