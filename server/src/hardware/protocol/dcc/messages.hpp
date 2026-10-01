/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2024-2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DCC_MESSAGES_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DCC_MESSAGES_HPP

#include <cstdint>
#include <cassert>
#ifndef NDEBUG
  #include "dcc.hpp"
#endif
#include "../../../utils/bit.hpp"

namespace DCC {

//! \see RCN-213 2.1
struct SetSimpleAccessory
{
  uint8_t data1;
  uint8_t data2;

  SetSimpleAccessory(uint16_t address, bool pair, bool enable)
  {
    // Format is: [10AAAAAA] [1AAADAAR]
    // where:
    // A = addressbit
    // D = enable/disable
    // R = 0=first/red 1=second/green
    //
    // See RCN-213: http://normen.railcommunity.de/RCN-213.pdf

    assert(address >= Accessory::addressMin && address <= Accessory::addressMax);

    address += 3; // User address 1 is address 4 in the command

    data1 = 0x80 | ((address >> 2) & 0x3F);
    data2 = ((~address >> 4) & 0x70) | ((address << 1) & 0x06) | 0x01; // Note: Address bits 8-10 are inverted!

    setBit<0>(data2, enable);
    setBit<3>(data2, pair);
  }
};
static_assert(sizeof(SetSimpleAccessory) == 2);

//! \see RCN-213 2.2
struct SetAdvancedAccessoryValue
{
  uint8_t address1;
  uint8_t address2;
  uint8_t value;

  SetAdvancedAccessoryValue(uint16_t address, uint8_t value_)
    : value{value_}
  {
    // Format is: [10AAAAAA] [0AAA0AA1] [DDDDDDDD] (3 bytes)
    // where:
    // A = address bit
    // D = data/aspect
    //
    // See RCN-213: http://normen.railcommunity.de/RCN-213.pdf

    assert(address >= Accessory::addressMin && address <= Accessory::addressMax);

    address += 3; // User address 1 is address 4 in the command

    address1 = 0x80 | ((address >> 2) & 0x3F);
    address2 = ((~address >> 4) & 0x70) | ((address << 1) & 0x06) | 0x01; // Note: Address bits 8-10 are inverted!
  }
};
static_assert(sizeof(SetAdvancedAccessoryValue) == 3);

}

#endif
