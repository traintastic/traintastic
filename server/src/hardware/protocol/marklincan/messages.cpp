/**
 * server/src/hardware/protocol/marklincan/messages.cpp
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

#include "messages.hpp"
#include "../../../utils/tohex.hpp"

namespace MarklinCAN {

std::string toString(const Message& message)
{
  std::string s;

  s.append("priority=").append(std::to_string(message.priority()));
  s.append(" command=").append(toHex(static_cast<uint8_t>(message.command())));
  s.append(" response=").append(message.isResponse() ? "1" : "0");
  s.append(" hash=").append(toHex(message.hash()));
  s.append(" dlc=").append(std::to_string(message.dlc));
  if(message.dlc > 0)
    s.append(" data=").append(toHex(message.data, message.dlc));

  return s;
}

}