/**
 * server/src/enum/loconetinterface.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_ENUM_LOCONETINTERFACETYPE_HPP
#define TRAINTASTIC_SERVER_ENUM_LOCONETINTERFACETYPE_HPP

#include <traintastic/enum/loconetinterfacetype.hpp>
#include <array>

inline constexpr std::array<LocoNetInterfaceType, 4> locoNetInterfaceTypeValues{{
  LocoNetInterfaceType::Serial,
  LocoNetInterfaceType::TCPBinary,
  LocoNetInterfaceType::LBServer,
  LocoNetInterfaceType::Z21,
}};

constexpr bool isSerial(LocoNetInterfaceType value)
{
  return (value == LocoNetInterfaceType::Serial);
}

constexpr bool isNetwork(LocoNetInterfaceType value)
{
  return
    value == LocoNetInterfaceType::TCPBinary ||
    value == LocoNetInterfaceType::LBServer ||
    value == LocoNetInterfaceType::Z21;
}

#endif
