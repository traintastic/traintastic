/**
 * server/src/hardware/protocol/xpressnet/utils.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2026 Filippo Gentile
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_XPRESSNET_UTILS_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_XPRESSNET_UTILS_HPP

#include <cstdint>

namespace XpressNet {

struct PendingQuery
{
  enum QueryType : uint8_t
  {
    LocoInfoAndF0F12 = 0,
    FuncInfoF13F28 = 1,
    FuncInfoF29F68= 2
  };

  uint16_t address = 0;
  QueryType type = QueryType::LocoInfoAndF0F12;
};

static constexpr uint8_t xbusVersionMajor(uint8_t versionHex)
{
  return (versionHex >> 4) & 0x0F;
}

static constexpr uint8_t xbusVersionMinor(uint8_t versionHex)
{
  return versionHex & 0x0F;
}

}

#endif
