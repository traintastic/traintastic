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
  if(inRange<uint32_t>(uid, Range::locomotiveDCC))
    return std::string("DCC(").append(std::to_string(uid & 0x3FFF)).append(")");

  return std::string("uid=").append(toHex(uid));
}

}
