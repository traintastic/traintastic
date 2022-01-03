/**
 * shared/src/enum/xpressnetserialinterfacetype.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_XPRESSNETSERIALINTERFACETYPE_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_XPRESSNETSERIALINTERFACETYPE_HPP

#include <cstdint>
#include "enum.hpp"

enum class XpressNetSerialInterfaceType : uint16_t
{
  LenzLI100 = 0,
  LenzLI100F = 1,
  LenzLI101F = 2,
  LenzLIUSB = 3,
  RoSoftS88XPressNetLI = 4,
  DigikeijsDR5000 = 5,
};

ENUM_NAME(XpressNetSerialInterfaceType, "xpressnet_serial_interface_type")

ENUM_VALUES(XpressNetSerialInterfaceType, 6,
{
  {XpressNetSerialInterfaceType::LenzLI100, "lenz_li100"},
  {XpressNetSerialInterfaceType::LenzLI100F, "lenz_li100f"},
  {XpressNetSerialInterfaceType::LenzLI101F, "lenz_li101f"},
  {XpressNetSerialInterfaceType::RoSoftS88XPressNetLI, "rosoft_s88xpressnetli"},
  {XpressNetSerialInterfaceType::LenzLIUSB, "lenz_liusb"},
  {XpressNetSerialInterfaceType::DigikeijsDR5000, "digikeijs_dr5000"},
})

#endif
