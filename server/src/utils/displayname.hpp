/**
 * server/src/utils/displayname.hpp
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

#ifndef TRAINTASTIC_SERVER_UTILS_DISPLAYNAME_HPP
#define TRAINTASTIC_SERVER_UTILS_DISPLAYNAME_HPP

#include <string_view>

namespace DisplayName
{
  namespace BoardTile
  {
    constexpr std::string_view outputMap = "board_tile:output_map";
  }
  namespace CommandStation
  {
    constexpr std::string_view emergencyStop = "command_station:emergency_stop";
    constexpr std::string_view online = "command_station:online";
    constexpr std::string_view powerOn = "command_station:power_on";
  }
  namespace Controller
  {
    constexpr std::string_view active = "controller:active";
    constexpr std::string_view commandStation = "controller:command_station";
  }
  namespace Hardware
  {
    constexpr std::string_view address = "hardware:address";
    constexpr std::string_view commandStation = "hardware:command_station";
    constexpr std::string_view decoders = "hardware:decoders";
    constexpr std::string_view inputMonitor = "hardware:input_monitor";
    constexpr std::string_view inputs = "hardware:inputs";
    constexpr std::string_view interface = "hardware:interface";
    constexpr std::string_view loconet = "hardware:loconet";
    constexpr std::string_view outputKeyboard = "hardware:output_keyboard";
    constexpr std::string_view outputs = "hardware:outputs";
    constexpr std::string_view speedSteps = "hardware:speed_steps";
    constexpr std::string_view xpressnet = "hardware:xpressnet";
  }
  namespace Interface
  {
    constexpr std::string_view status = "interface:status";
  }
  namespace IP
  {
    constexpr std::string_view hostname = "ip:hostname";
    constexpr std::string_view port = "ip:port";
  }
  namespace List
  {
    constexpr std::string_view add = "list:add";
    constexpr std::string_view moveUp = "list:move_up";
    constexpr std::string_view moveDown = "list:move_down";
    constexpr std::string_view remove = "list:remove";
  }
  namespace Object
  {
    constexpr std::string_view id = "object:id";
    constexpr std::string_view name = "object:name";
    constexpr std::string_view notes = "object:notes";
  }
  namespace Serial
  {
    constexpr std::string_view device = "serial:device";
    constexpr std::string_view baudrate = "serial:baudrate";
    constexpr std::string_view flowControl = "serial:flow_control";
  }
  namespace Vehicle
  {
    namespace Rail
    {
      constexpr std::string_view lob = "vehicle.rail:lob";
      constexpr std::string_view speedMax = "vehicle.rail:speed_max";
      constexpr std::string_view decoder = "vehicle.rail:decoder";
      constexpr std::string_view totalWeight = "vehicle.rail:total_weight";
      constexpr std::string_view train = "vehicle.rail:train";
      constexpr std::string_view weight = "vehicle.rail:weight";
    }
  }
  namespace World
  {
    constexpr std::string_view controllers = "world:controllers";
    constexpr std::string_view decoders = "world:decoders";
    constexpr std::string_view uuid = "world:uuid";
  }
}

#endif
