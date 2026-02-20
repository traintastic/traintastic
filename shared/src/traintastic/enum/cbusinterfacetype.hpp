/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_CBUSINTERFACETYPE_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_CBUSINTERFACETYPE_HPP

#include <cstdint>
#include <array>
#include "enum.hpp"

enum class CBUSInterfaceType : uint8_t
{
  CANUSB = 0,
  CANEther = 1,
};

TRAINTASTIC_ENUM(CBUSInterfaceType, "cbus_interface_type", 2,
{
  {CBUSInterfaceType::CANUSB, "canusb"},
  {CBUSInterfaceType::CANEther, "canether"},
});

inline constexpr std::array<CBUSInterfaceType, 2> CBUSInterfaceTypeValues{{
  CBUSInterfaceType::CANUSB,
  CBUSInterfaceType::CANEther,
}};

#endif
