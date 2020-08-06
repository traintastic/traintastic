/**
 * server/src/enum/loconetserialinterface.hpp
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

#ifndef TRAINTASTIC_SERVER_ENUM_LOCONETSERIALINTERFACE_HPP
#define TRAINTASTIC_SERVER_ENUM_LOCONETSERIALINTERFACE_HPP

#include <traintastic/enum/loconetserialinterface.hpp>
#include <nlohmann/json.hpp>
#ifndef DISABLE_LUA_SCRIPTING
  #include "../lua/enumvalues.hpp"
#endif

inline constexpr std::array<LocoNetSerialInterface, 3> LocoNetSerialInterfaceValues{{
  LocoNetSerialInterface::Custom,
  LocoNetSerialInterface::DigikeijsDR5000,
  LocoNetSerialInterface::RoSoftLocoNetInterface,
}};

#ifndef DISABLE_LUA_SCRIPTING
LUA_ENUM_VALUES(LocoNetSerialInterface, 3,
{
  {LocoNetSerialInterface::Custom, "CUSTOM"},
  {LocoNetSerialInterface::DigikeijsDR5000, "DIGIKEIJS_DR5000"},
  {LocoNetSerialInterface::RoSoftLocoNetInterface, "ROSOFT_LOCONET_INTERFACE"},
})
#endif

#endif
