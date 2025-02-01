/**
 * server/src/hardware/protocol/withrottle/messages.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022,2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_WITHROTTLE_MESSAGES_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_WITHROTTLE_MESSAGES_HPP

#include <string>
#include <string_view>
#include <span>
#include <version.hpp>
#include <traintastic/enum/direction.hpp>

namespace WiThrottle {

enum class ThrottleCommand : char
{
  Consist = 'C',
  ConsistLeadFromRosterEntry = 'c',
  Dispatch = 'd',
  SetAddressFromRosterEntry = 'E',
  Function = 'F',
  ForceFunction = 'f',
  Idle = 'I',
  SetLongAddress = 'L',
  MomentaryFunction = 'm',
  Query = 'q',
  Quit = 'Q',
  SetDirection = 'R',
  Release = 'r',
  SetShortAddress = 'S',
  SetSpeedStepMode = 's',
  SetSpeed = 'V',
  EmergencyStop = 'X',
};

struct RosterListEntry
{
  std::string_view name;
  uint16_t address;
  bool isLongAddress;
};

constexpr uint8_t speedMax = 126;

constexpr std::string_view protocolVersion()
{
  return "VN2.0";
}

inline std::string rosterList(std::span<const RosterListEntry> list)
{
  std::string s{"RL"};
  s.append(std::to_string(list.size()));
  for(const auto& entry : list)
  {
    s.append("]\\[");
    s.append(entry.name); //! \todo what if name contains ]\\[ or }|{ ??
    s.append("}|{");
    s.append(std::to_string(entry.address));
    s.append("}|{");
    s.append(entry.isLongAddress ? "L" : "S");
  }
  return s;
}

constexpr std::string_view trackPowerOff()
{
  return "PPA0";
}

constexpr std::string_view trackPowerOn()
{
  return "PPA1";
}

constexpr std::string_view trackPowerUnknown()
{
  return "PPA2";
}

constexpr std::string_view trackPower(bool on)
{
  return on ? trackPowerOn() : trackPowerOff();
}

constexpr std::string_view trackPower(TriState on)
{
  switch(on)
  {
    case TriState::False:
      return trackPowerOff();

    case TriState::True:
      return trackPowerOn();

    case TriState::Undefined:
      break;
  }
  return trackPowerUnknown();
}

inline std::string throttleCommand(char multiThrottleId, char command, uint16_t address, bool isLongAddress)
{
  std::string s("M");
  s.push_back(multiThrottleId);
  s.push_back(command);
  s.push_back(isLongAddress ? 'L' : 'S');
  s.append(std::to_string(address));
  s.append("<;>");
  return s;
}

inline std::string throttleRelease(char multiThrottleId)
{
  std::string s("M");
  s.push_back(multiThrottleId);
  s.append("-*<;>r");
  return s;
}

inline std::string throttleRelease(char multiThrottleId, uint16_t address, bool isLongAddress)
{
  return throttleCommand(multiThrottleId, '-', address, isLongAddress).append("r");
}

inline std::string throttleSteal(char multiThrottleId, uint16_t address, bool isLongAddress)
{
  return
    throttleCommand(multiThrottleId, 'S', address, isLongAddress)
      .append(isLongAddress ? "L" : "S")
      .append(std::to_string(address));
}

inline std::string throttleChange(char multiThrottleId, uint16_t address, bool isLongAddress)
{
  return throttleCommand(multiThrottleId, 'A', address, isLongAddress);
}

inline std::string throttleFuctionNames(char multiThrottleId, uint16_t address, bool isLongAddress, const std::unordered_map<uint32_t, std::string_view>& functionNames)
{
  std::string s{throttleCommand(multiThrottleId, 'L', address, isLongAddress)};
  s.append("]\\[");
  for(uint32_t i = 0; i <= 28; ++i)
  {
    if(auto it = functionNames.find(i); it != functionNames.end())
      s.append(it->second);
    s.append("]\\[");
  }
  return s;
}

inline std::string throttleFunction(char multiThrottleId, uint16_t address, bool isLongAddress, uint32_t functionNumber, bool state)
{
  assert(functionNumber <= 28);
  return
    throttleChange(multiThrottleId, address, isLongAddress)
      .append(state ? "F1" : "F0")
      .append(std::to_string(functionNumber));
}

inline std::string throttleEstop(char multiThrottleId, uint16_t address, bool isLongAddress)
{
  return
    throttleChange(multiThrottleId, address, isLongAddress)
      .append("V-1");
}

inline std::string throttleSpeed(char multiThrottleId, uint16_t address, bool isLongAddress, uint8_t speed)
{
  assert(speed <= 126);
  return
    throttleChange(multiThrottleId, address, isLongAddress)
      .append("V")
      .append(std::to_string(speed));
}

inline std::string throttleDirection(char multiThrottleId, uint16_t address, bool isLongAddress, Direction direction)
{
  assert(direction == Direction::Forward || direction == Direction::Reverse);
  return
    throttleChange(multiThrottleId, address, isLongAddress)
      .append(direction == Direction::Reverse ? "R0" : "R1");
}

inline std::string throttleSpeedStepMode(char multiThrottleId, uint16_t address, bool isLongAddress, uint8_t speedSteps)
{
  std::string s{throttleChange(multiThrottleId, address, isLongAddress)};
  switch(speedSteps)
  {
    case 14:
      s.append("s8");
      break;

    case 27:
      s.append("s4");
      break;

    case 28:
      s.append("s2");
      break;

    case 126:
    case 128:
    default:
      assert(speedSteps == 126 || speedSteps == 128);
      s.append("s1");
      break;
  }
  return s;
}

inline std::string fastClock(uint64_t secondsSinceEpoch, uint8_t rate)
{
  return std::string("PFT").append(std::to_string(secondsSinceEpoch)).append("<;>").append(std::to_string(rate));
}

inline std::string alert(std::string_view message)
{
  return std::string("HM").append(message);
}

inline std::string info(std::string_view message)
{
  return std::string("Hm").append(message);
}

constexpr std::string_view serverType()
{
  return "HTTraintastic";
}

constexpr std::string_view serverVersion()
{
  return "HtTraintastic v" TRAINTASTIC_VERSION_FULL;
}


constexpr std::string_view quit()
{
  return "Q";
}

}

#endif
