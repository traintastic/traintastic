/**
 * server/src/hardware/protocol/marklincan/uid.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#include "uid.hpp"
#include "../../../utils/inrange.hpp"
#include "../../../utils/tohex.hpp"

namespace MarklinCAN::UID {

std::string toString(uint32_t uid)
{
  if(inRange(uid, Range::locomotiveMotorola))
    return std::string("Motorola(").append(std::to_string(uid - Range::locomotiveMotorola.first)).append(")");
  if(inRange(uid, Range::locomotiveMFX))
    return std::string("MFX(").append(std::to_string(uid - Range::locomotiveMFX.first)).append(")");
  if(inRange<uint32_t>(uid, Range::locomotiveDCC))
    return std::string("DCC(").append(std::to_string(uid - Range::locomotiveDCC.first)).append(")");

  if(inRange(uid, Range::accessoryMotorola))
    return std::string("Motorola(").append(std::to_string(uid - Range::accessoryMotorola.first + 1)).append(")");
  if(inRange(uid, Range::accessoryDCC))
    return std::string("DCC(").append(std::to_string(uid - Range::accessoryDCC.first + 1)).append(")");
  if(inRange(uid, Range::accessorySX1))
    return std::string("SX1(").append(std::to_string(uid - Range::accessorySX1.first + 1)).append(")");

  return std::string("uid=").append(toHex(uid));
}

}
