/**
 * shared/src/enum/xpressnetserialinterface.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_XPRESSNETSERIALINTERFACE_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_XPRESSNETSERIALINTERFACE_HPP

#include <cstdint>
#include "enum.hpp"

enum class XpressNetSerialInterface : uint16_t
{
  Custom = 0,
  LenzLI100 = 1,
  LenzLI100F = 2,
  LenzLI101F = 3,
  RoSoftS88XPressNetLI = 4,
};

template<>
struct EnumName<XpressNetSerialInterface>
{
  static constexpr char const* value = "xpressnet_serial_interface";
};

#endif
