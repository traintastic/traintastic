/**
 * server/src/hardware/protocol/ecos/messages.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_MESSAGES_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_MESSAGES_HPP

#include <string>

namespace ECoS {

struct ObjectId
{
  static constexpr uint16_t ecos = 1;
  static constexpr uint16_t programmingTrack = 5;
  static constexpr uint16_t locManager = 10;
  static constexpr uint16_t switchManager = 11;
  static constexpr uint16_t shuttleTrainControl = 12;
  static constexpr uint16_t deviceManager = 20;
  static constexpr uint16_t sniffer = 25;
  static constexpr uint16_t feedbackManager = 26;
  static constexpr uint16_t booster = 27;
  static constexpr uint16_t controlDesk = 31;
};

struct Option
{
  static constexpr std::string_view addr = "addr";
  static constexpr std::string_view go = "go";
  static constexpr std::string_view info = "info";
  static constexpr std::string_view name = "name";
  static constexpr std::string_view protocol = "protocol";
  static constexpr std::string_view status = "status";
  static constexpr std::string_view stop = "stop";
  static constexpr std::string_view view = "view";
};

inline std::string buildCommand(std::string_view command, uint16_t objectId, std::initializer_list<std::string_view> options)
{
  std::string s(command);
  s.append("(").append(std::to_string(objectId));
  for(auto option : options)
    s.append(", ").append(option);
  s.append(")\n");
  return s;
}

inline std::string queryObjects(uint16_t objectId, std::initializer_list<std::string_view> options = {})
{
  return buildCommand("queryObjects", objectId, options);
}

inline std::string set(uint16_t objectId, std::initializer_list<std::string_view> options)
{
  return buildCommand("set", objectId, options);
}

inline std::string get(uint16_t objectId, std::initializer_list<std::string_view> options)
{
  return buildCommand("get", objectId, options);
}

inline std::string create(uint16_t objectId, std::initializer_list<std::string_view> options)
{
  return buildCommand("create", objectId, options);
}

inline std::string delete_(uint16_t objectId, std::initializer_list<std::string_view> options)
{
  return buildCommand("delete", objectId, options);
}

inline std::string request(uint16_t objectId, std::initializer_list<std::string_view> options)
{
  return buildCommand("request", objectId, options);
}

inline std::string release(uint16_t objectId, std::initializer_list<std::string_view> options)
{
  return buildCommand("release", objectId, options);
}

}

#endif
