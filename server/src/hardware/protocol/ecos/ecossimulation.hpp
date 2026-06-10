/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2022-2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_ECOSSIMULATION_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_ECOSSIMULATION_HPP

#include <vector>
#include <string>
#include <cstdint>
#include "object/ecoslocomotiveprotocol.hpp"

namespace ECoS {

struct Simulation
{
  struct Object
  {
    uint16_t id;
  };

  struct ECoS
  {
    std::string commandStationType;
    std::string protocolVersion;
    std::string hardwareVersion;
    std::string applicationVersion;
    std::string applicationVersionSuffix;
    bool railcom;
    bool railcomPlus;

    void clear()
    {
      commandStationType.clear();
      protocolVersion.clear();
      hardwareVersion.clear();
      applicationVersion.clear();
      applicationVersionSuffix.clear();
      railcom = false;
      railcomPlus = false;
    }
  };

  struct Locomotive : Object
  {
    LocomotiveProtocol protocol;
    uint16_t address;
  };

  struct Switch : Object
  {
    std::string name1;
    std::string name2;
    std::string name3;
    uint16_t address;
    std::string addrext;
    std::string type;
    int symbol;
    std::string protocol;
    uint8_t state;
    std::string mode;
    uint16_t duration;
    uint8_t variant;
  };

  struct S88 : Object
  {
    uint8_t ports;
  };

  struct ECoSDetector : Object
  {
    uint8_t ports;
    bool railcom;
  };

  ECoS ecos;
  std::vector<Locomotive> locomotives;
  std::vector<Switch> switches;
  std::vector<S88> s88;
  std::vector<ECoSDetector> ecosDetector;

  void clear()
  {
    ecos.clear();
    locomotives.clear();
    switches.clear();
    s88.clear();
    ecosDetector.clear();
  }
};

}

#endif
