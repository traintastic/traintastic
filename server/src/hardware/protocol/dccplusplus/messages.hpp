/**
 * server/src/hardware/protocol/dccplusplus/messages.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DCCPLUSPLUS_MESSAGES_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DCCPLUSPLUS_MESSAGES_HPP

#include <string_view>
#include <traintastic/enum/direction.hpp>

namespace DCCPlusPlus {

namespace Ex {
  // see: https://dcc-ex.com/reference/software/command-reference.html

  enum class Track {
    Main,
    Programming,
  };

  //! Turn Power OFF to tracks (Both Main & Programming)
  constexpr std::string_view powerOff()
  {
    return "<0>\n";
  }

  constexpr std::string_view powerOff(Track track)
  {
    switch(track)
    {
      case Track::Main:
        return "<0 MAIN>\n";

      case Track::Programming:
        return "<0 PROG>\n";
    }
    return "";
  }

  //! Turn Power ON to tracks (Both Main & Programming)
  constexpr std::string_view powerOn()
  {
    return "<1>\n";
  }

  constexpr std::string_view powerOn(Track track)
  {
    switch(track)
    {
      case Track::Main:
        return "<1 MAIN>\n";

      case Track::Programming:
        return "<1 PROG>\n";
    }
    return "";
  }

  constexpr std::string_view powerOnJoin()
  {
    return "<1 JOIN>\n";
  }

  //! Displays the instantaneous current on the MAIN Track
  constexpr std::string_view getMainTrackCurrent()
  {
    return "<c>\n";
  }

  //! Command to Store definitions to EEPROM
  constexpr std::string_view eepromStore()
  {
    return "<E>\n";
  }

  //! Command to Erase ALL (turnouts, sensors, and outputs) definitions from EEPROM
  constexpr std::string_view eepromEraseAll()
  {
    return "<e>\n";
  }

  //! Lists Status of all sensors
  constexpr std::string_view getSensorStatuses()
  {
    return "<Q>\n";
  }

  //! Read Loco address (programming track only)
  constexpr std::string_view readLocoAddress()
  {
    return "<R>\n";
  }

  //! Lists all defined sensors.
  constexpr std::string_view getSensorList()
  {
    return "<S>\n";
  }

  //! DCC++ EX CommandStation Status
  constexpr std::string_view getStatus()
  {
    return "<s>\n";
  }

  //! Lists all defined turnouts
  constexpr std::string_view getTurnoutList()
  {
    return "<T>\n";
  }

  //! Lists all defined output pins
  constexpr std::string_view getOutputPinList()
  {
    return "<Z>\n";
  }

  //! Stops all locos on the track but leaves power on.
  constexpr std::string_view emergencyStop()
  {
    return "<!>\n";
  }

  inline std::string setLocoSpeedAndDirection(uint16_t address, uint8_t speed, bool emergencyStop, Direction direction)
  {
    assert(address <= 10293);
    assert(speed <= 126);

    return std::string("<t ")
      .append(std::to_string(address))
      .append(" ")
      .append(emergencyStop ? "-1" : std::to_string(speed))
      .append(direction == Direction::Forward ? " 1" : " 0")
      .append(">\n");
  }

  constexpr std::string_view forgetLocos()
  {
    return "<->\n";
  }

  inline std::string forgetLoco(uint16_t address)
  {
    assert(address <= 10293);

    return std::string("<- ")
      .append(std::to_string(address))
      .append(">\n");
  }

  inline std::string setLocoFunction(uint16_t address, uint8_t function, bool value)
  {
    assert(address <= 10293);
    assert(function <= 68);

    return std::string("<F ")
      .append(std::to_string(address))
      .append(" ")
      .append(std::to_string(function))
      .append(value ? " 1" : " 0")
      .append(">\n");
  }

  inline std::string setAccessory(uint16_t linearAddress, bool activate)
  {
    assert(linearAddress >= 1 && linearAddress <= 2044);

    return std::string("<a ")
      .append(std::to_string(linearAddress))
      .append(activate ? " 1" : " 0")
      .append(">\n");
  }

  inline std::string setAccessory(uint16_t address, uint8_t subAddress, bool activate)
  {
    assert(address <= 511);
    assert(subAddress <= 3);

    return std::string("<a ")
      .append(std::to_string(address))
      .append(" ")
      .append(std::to_string(subAddress))
      .append(activate ? " 1" : " 0")
      .append(">\n");
  }

  inline std::string_view setSpeedSteps(uint8_t value)
  {
    switch(value)
    {
      case 28:
        return "<D SPEED28>\n";
      case 128:
        return "<D SPEED128>\n";
    }
    return "";
  }
}

}

#endif
