/**
 * server/src/enum/xpressnetserialinterface.hpp
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

#ifndef TRAINTASTIC_SERVER_ENUM_XPRESSNETSERIALINTERFACE_HPP
#define TRAINTASTIC_SERVER_ENUM_XPRESSNETSERIALINTERFACE_HPP

#include <traintastic/enum/xpressnetserialinterface.hpp>
#include <nlohmann/json.hpp>

inline constexpr std::array<XpressNetSerialInterface, 5> XpressNetSerialInterfaceValues{{
  XpressNetSerialInterface::Custom,
  XpressNetSerialInterface::LenzLI100,
  XpressNetSerialInterface::LenzLI100F,
  XpressNetSerialInterface::LenzLI101F,
  XpressNetSerialInterface::RoSoftS88XPressNetLI,
}};

NLOHMANN_JSON_SERIALIZE_ENUM(XpressNetSerialInterface,
{
  {XpressNetSerialInterface::Custom, "custom"},
  {XpressNetSerialInterface::LenzLI100, "lenz_li100"},
  {XpressNetSerialInterface::LenzLI100F, "lenz_li100f"},
  {XpressNetSerialInterface::LenzLI101F, "lenz_li101f"},
  {XpressNetSerialInterface::RoSoftS88XPressNetLI, "rosoft_s88xpressnetli"},
})

#endif
