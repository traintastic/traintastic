/**
 * server/src/hardware/protocol/ecos/object/locomotiveprotocol.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_OBJECT_LOCOMOTIVEPROTOCOL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_OBJECT_LOCOMOTIVEPROTOCOL_HPP

#include <string_view>

namespace ECoS
{

enum class LocomotiveProtocol
{
  Unknown = 0,
  MM14 = 1,
  MM27 = 2,
  MM28 = 3,
  DCC14 = 4,
  DCC28 = 5,
  DCC128 = 6,
  SX32 = 7,
  MMFKT = 8,
};

constexpr std::string_view toString(LocomotiveProtocol protocol)
{
  switch(protocol)
  {
    case LocomotiveProtocol::MM14:
      return "MM14";

    case LocomotiveProtocol::MM27:
      return "MM27";

    case LocomotiveProtocol::MM28:
      return "MM28";

    case LocomotiveProtocol::DCC14:
      return "DCC14";

    case LocomotiveProtocol::DCC28:
      return "DCC28";

    case LocomotiveProtocol::DCC128:
      return "DCC128";

    case LocomotiveProtocol::SX32:
      return "SX32";

    case LocomotiveProtocol::MMFKT:
      return "MMFKT";

    case LocomotiveProtocol::Unknown:
      break;
  }
  return {};
}

inline bool fromString(std::string_view text, LocomotiveProtocol& protocol)
{
  if(text == "MM14")
    protocol = LocomotiveProtocol::MM14;
  else if(text == "MM27")
    protocol = LocomotiveProtocol::MM27;
  else if(text == "MM28")
    protocol = LocomotiveProtocol::MM28;
  else if(text == "DCC14")
    protocol = LocomotiveProtocol::DCC14;
  else if(text == "DCC28")
    protocol = LocomotiveProtocol::DCC28;
  else if(text == "DCC128")
    protocol = LocomotiveProtocol::DCC128;
  else if(text == "SX32")
    protocol = LocomotiveProtocol::SX32;
  else if(text == "MMFKT")
    protocol = LocomotiveProtocol::MMFKT;
  else
    return false;
  return true;
}

}

#endif
