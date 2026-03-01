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

#include "cbuscancmd.hpp"
#include "../../cbusmessages.hpp"

namespace CBUS::Module {

CANCMD::CANCMD(Simulator& simulator, uint16_t nodeNumber_, uint8_t canId_)
  : CANModule(simulator, nodeNumber_, canId_)
{
}

void CANCMD::receive(const Message& message)
{
  switch(message.opCode)
  {
    using enum OpCode;

    case RTOF:
      m_trackOn = false;
      send(TrackOff());
      break;

    case RTON:
      m_trackOn = true;
      send(TrackOn());
      break;

    default:
      break;
  }
}

}
