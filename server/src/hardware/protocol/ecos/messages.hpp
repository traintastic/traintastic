/**
 * server/src/hardware/protocol/ecos/messages.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022,2024 Reinder Feenstra
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
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace ECoS {

struct Command
{
  static constexpr std::string_view queryObjects = "queryObjects";
  static constexpr std::string_view set = "set";
  static constexpr std::string_view get = "get";
  static constexpr std::string_view create = "create";
  static constexpr std::string_view delete_ = "delete";
  static constexpr std::string_view request = "request";
  static constexpr std::string_view release = "release";
};

struct ObjectId
{
  static constexpr uint16_t ecos = 1;
  static constexpr uint16_t programmingTrack = 5;
  // unknown object: 7
  static constexpr uint16_t locomotiveManager = 10;
  static constexpr uint16_t switchManager = 11;
  static constexpr uint16_t shuttleTrainControl = 12;
  // unknown object: 19
  static constexpr uint16_t deviceManager = 20;
  static constexpr uint16_t sniffer = 25;
  static constexpr uint16_t feedbackManager = 26;
  static constexpr uint16_t booster = 27;
  static constexpr uint16_t controlDesk = 31;
  // unknown object: 40
  static constexpr uint16_t s88 = 100;
  static constexpr uint16_t ecosDetector = 200;

  static constexpr uint16_t switchMin = 20000;
  static constexpr uint16_t switchMax = 29999; // unsure, guessed it, turntable starts at 30000
  static constexpr uint16_t turntableMin = 30000;
  static constexpr uint16_t turntableMax = 39999; // unsure, guessed it, same range as switch
};

struct Option
{
  static constexpr std::string_view addr = "addr";
  static constexpr std::string_view addrext = "addrext";
  static constexpr std::string_view applicationversion = "applicationversion";
  static constexpr std::string_view applicationversionsuffix = "applicationversionsuffix";
  static constexpr std::string_view commandstationtype = "commandstationtype";
  static constexpr std::string_view control = "control";
  static constexpr std::string_view dir = "dir";
  static constexpr std::string_view duration = "duration";
  static constexpr std::string_view force = "force";
  static constexpr std::string_view func = "func";
  static constexpr std::string_view go = "go";
  static constexpr std::string_view hardwareversion = "hardwareversion";
  static constexpr std::string_view mode = "mode";
  static constexpr std::string_view name = "name";
  static constexpr std::string_view name1 = "name1";
  static constexpr std::string_view name2 = "name2";
  static constexpr std::string_view name3 = "name3";
  static constexpr std::string_view ports = "ports";
  static constexpr std::string_view protocol = "protocol";
  static constexpr std::string_view protocolversion = "protocolversion";
  static constexpr std::string_view railcom = "railcom";
  static constexpr std::string_view railcomplus = "railcomplus";
  static constexpr std::string_view speedStep = "speedstep";
  static constexpr std::string_view state = "state";
  static constexpr std::string_view status = "status";
  static constexpr std::string_view stop = "stop";
  static constexpr std::string_view switch_ = "switch";
  static constexpr std::string_view symbol = "symbol";
  static constexpr std::string_view type = "type";
  static constexpr std::string_view variant = "variant";
  static constexpr std::string_view view = "view";
};

enum class Status : uint32_t
{
  Ok = 0,
  UnknownOption = 11,
  UnknownObject = 15,
  NoManagerObject = 22,
};

struct Request
{
  std::string_view command;
  uint16_t objectId;
  std::vector<std::string_view> options;
};

struct Reply
{
  std::string_view command;
  uint16_t objectId;
  std::vector<std::string_view> options;
  std::vector<std::string_view> lines;
  Status status;
  std::string_view statusMessage;
};

struct Event
{
  uint16_t objectId;
  std::vector<std::string_view> lines;
  Status status;
  std::string_view statusMessage;
};

struct Line
{
  uint16_t objectId;
  std::unordered_map<std::string_view, std::string_view> values;
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
  return buildCommand(Command::queryObjects, objectId, options);
}

inline std::string set(uint16_t objectId, std::initializer_list<std::string_view> options)
{
  return buildCommand(Command::set, objectId, options);
}

template<class T>
inline std::string set(uint16_t objectId, std::string_view option, T value)
{
  std::string s(Command::set);
  s.append("(").append(std::to_string(objectId));
  s.append(", ").append(option);
  s.append("[");
  if constexpr(std::is_same_v<T, std::string>)
    s.append(value);
  else
    s.append(std::to_string(value));
  s.append("]");
  s.append(")\n");
  return s;
}

template<class T1, class T2>
inline std::string set(uint16_t objectId, std::string_view option, T1 value1, T2 value2)
{
  std::string s(Command::set);
  s.append("(").append(std::to_string(objectId));
  s.append(", ").append(option);
  s.append("[").append(std::to_string(value1)).append(",").append(std::to_string(value2)).append("]");
  s.append(")\n");
  return s;
}

inline std::string get(uint16_t objectId, std::initializer_list<std::string_view> options)
{
  return buildCommand(Command::get, objectId, options);
}

template<class T>
inline std::string get(uint16_t objectId, std::string_view option, T value)
{
  std::string s(Command::get);
  s.append("(").append(std::to_string(objectId));
  s.append(", ").append(option);
  s.append("[").append(std::to_string(value)).append("]");
  s.append(")\n");
  return s;
}

inline std::string create(uint16_t objectId, std::initializer_list<std::string_view> options)
{
  return buildCommand(Command::create, objectId, options);
}

inline std::string delete_(uint16_t objectId, std::initializer_list<std::string_view> options)
{
  return buildCommand(Command::delete_, objectId, options);
}

inline std::string request(uint16_t objectId, std::initializer_list<std::string_view> options)
{
  return buildCommand(Command::request, objectId, options);
}

inline std::string release(uint16_t objectId, std::initializer_list<std::string_view> options)
{
  return buildCommand(Command::release, objectId, options);
}

bool parseRequest(std::string_view message, Request& request);

bool isReply(std::string_view message);
bool parseReply(std::string_view message, Reply& reply);
bool parseEvent(std::string_view message, Event& event);

bool parseId(std::string_view line, uint16_t& id);
bool parseLine(std::string_view text, Line& line);

bool parseOptionValue(std::string_view text, std::string_view& option, std::string_view& value);

constexpr bool isS88FeedbackId(uint16_t id)
{
  return id >= ObjectId::s88 && id < ObjectId::ecosDetector;
}

}

#endif
