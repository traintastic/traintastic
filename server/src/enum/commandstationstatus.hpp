/**
 * server/src/enum/commandstationstatus.hpp
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

#ifndef TRAINTASTIC_SERVER_ENUM_COMMANDSTATIONSTATUS_HPP
#define TRAINTASTIC_SERVER_ENUM_COMMANDSTATIONSTATUS_HPP

#include <nlohmann/json.hpp>
#include <traintastic/enum/commandstationstatus.hpp>
#ifndef DISABLE_LUA_SCRIPTING
  #include "../lua/enumvalues.hpp"
#endif

#ifndef DISABLE_LUA_SCRIPTING
LUA_ENUM_VALUES(CommandStationStatus, 4,
{
  {CommandStationStatus::Offline, "OFFLINE"},
  {CommandStationStatus::Initializing, "INITIALIZING"},
  {CommandStationStatus::Online, "ONLINE"},
  {CommandStationStatus::Error, "ERROR"},
})
#endif

#endif
