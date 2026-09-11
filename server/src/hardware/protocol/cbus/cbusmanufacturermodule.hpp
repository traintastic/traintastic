/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_CBUSMANUFACTURERMODULE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_CBUSMANUFACTURERMODULE_HPP

#include <cstdint>
#include <string_view>

namespace CBUS {

// see: https://github.com/MERG-DEV/cbusdefs/tree/master

constexpr uint8_t manufacturerDevelopment = 13;
constexpr uint8_t manufacturerSPROG = 44;
constexpr uint8_t manufacturerRocRail = 70;
constexpr uint8_t manufacturerSpectrumEngineering = 80;
constexpr uint8_t manufacturerMERG = 165;
constexpr uint8_t manufacturerRME = 248;
constexpr uint8_t manufacturerSyspixie = 249;

constexpr std::string_view manufacturerName(uint8_t manufacturerId)
{
  switch(manufacturerId)
  {
    case manufacturerDevelopment:
      return "Development";

    case manufacturerSPROG:
      return "SPROG";

    case manufacturerRocRail:
      return "RocRail";

    case manufacturerSpectrumEngineering:
      return "Spectrum Engineering";

    case manufacturerMERG:
      return "MERG";

    case manufacturerRME:
      return "RME";

    case manufacturerSyspixie:
      return "Syspixie";
  }
  return {};
}

constexpr std::string_view moduleNameMERG(uint8_t moduleId)
{
  switch(moduleId)
  {
    case 0:
      return "SLIM";

    case 1:
      return "CANACC4";

    case 2:
      return "CANACC5";

    case 3:
      return "CANACC8";

    case 4:
      return "CANACE3";

    case 5:
      return "CANACE8C";

    case 6:
      return "CANLED";

    case 7:
      return "CANLED64";

    case 8:
      return "CANACC4_2";

    case 9:
      return "CANCAB";

    case 10:
      return "CANCMD";

    case 11:
      return "CANSERVO";

    case 12:
      return "CANBC";

    case 13:
      return "CANRPI";

    case 14:
      return "CANTTCA";

    case 15:
      return "CANTTCB";

    case 16:
      return "CANHS";

    case 17:
      return "CANTOTI";

    case 18:
      return "CAN8I8O";

    case 19:
      return "CANSERVO8C";

    case 20:
      return "CANRFID";

    case 21:
      return "CANTC4";

    case 22:
      return "CANACE16C";

    case 23:
      return "CANIO8";

    case 24:
      return "CANSNDX";

    case 25:
      return "CANEther";

    case 26:
      return "CANSIG64";

    case 27:
      return "CANSIG8";

    case 28:
      return "CANCOND8C";

    case 29:
      return "CANPAN";

    case 30:
      return "CANACE3C";

    case 31:
      return "CANPanel";

    case 32:
      return "CANMIO";

    case 33:
      return "CANACE8MIO";

    case 34:
      return "CANSOL";

    case 35:
      return "CANBIP";

    case 36:
      return "CANCDU";

    case 37:
      return "CANACC4CDU";

    case 38:
      return "CANWiBase";

    case 39:
      return "WiCAB";

    case 40:
      return "CANWiFi";

    case 41:
      return "CANFTT";

    case 42:
      return "CANHNDST";

    case 43:
      return "CANTCHNDST";

    case 44:
      return "CANRFID8";

    case 45:
      return "CANmchRFID";

    case 46:
      return "CANPiWi";

    case 47:
      return "CAN4DC";

    case 48:
      return "CANELEV";

    case 49:
      return "CANSCAN";

    case 50:
      return "CANMIO_SVO";

    case 51:
      return "CANMIO_INP";

    case 52:
      return "CANMIO_OUT";

    case 53:
      return "CANBIP_OUT";

    case 54:
      return "CANASTOP";

    case 55:
      return "CANCSB";

    case 56:
      return "CANMAG";

    case 57:
      return "CANACE16CMIO";

    case 58:
      return "CANPiNODE";

    case 59:
      return "CANDISP";

    case 60:
      return "CANCOMPUTE";

    case 61:
      return "CANRC522";

    case 62:
      return "CANINP";

    case 63:
      return "CANOUT";

    case 64:
      return "CANEMIO";

    case 65:
      return "CANCABDC";

    case 66:
      return "CANRCOM";

    case 67:
      return "CANMP3";

    case 68:
      return "CANXMAS";

    case 69:
      return "CANSVOSET";

    case 70:
      return "CANCMDDC";

    case 71:
      return "CANTEXT";

    case 72:
      return "CANASIGNAL";

    case 73:
      return "CANSLIDER";

    case 74:
      return "CANDCATC";

    case 75:
      return "CANGATE";

    case 76:
      return "CANSINP";

    case 77:
      return "CANSOUT";

    case 78:
      return "CANSBIP";

    case 79:
      return "CANBUFFER";

    case 80:
      return "CANLEVER";

    case 81:
      return "CANSHIELD";

    case 82:
      return "CAN4IN4OUT";

    case 83:
      return "CANCMDB";

    case 84:
      return "CANPIXEL";

    case 85:
      return "CANCABPE";

    case 86:
      return "CANSMARTTD";

    case 87:
      return "CANARGB";

    case 0xFC:
      return "VLCB";
  }
  return {};
}

constexpr std::string_view moduleName(uint8_t manufacturerId, uint8_t moduleId)
{
  switch(manufacturerId)
  {
    case manufacturerMERG:
      return moduleNameMERG(moduleId);
  }
  return {};
}

}

#endif
