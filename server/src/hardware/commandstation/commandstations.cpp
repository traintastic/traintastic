/**
 * server/src/hardware/commandstation/commandstations.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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

#include "commandstations.hpp"

std::shared_ptr<CommandStation> CommandStations::create(const std::weak_ptr<World>& world, std::string_view classId, std::string_view id)
{
  if(classId == DCCPlusPlusSerial::classId)
    return DCCPlusPlusSerial::create(world, id);
  if(classId == LocoNetSerial::classId)
    return LocoNetSerial::create(world, id);
  else if(classId == LocoNetTCPBinary::classId)
    return LocoNetTCPBinary::create(world, id);
#ifdef USB_XPRESSNET
  else if(classId == USBXpressNetInterface::classId)
    return USBXpressNetInterface::create(world, id);
#endif
  else if(classId == XpressNetSerial::classId)
    return XpressNetSerial::create(world, id);
  else if(classId == RocoZ21::classId)
    return RocoZ21::create(world, id);
  else if(classId == VirtualCommandStation::classId)
    return VirtualCommandStation::create(world, id);
  else
    return std::shared_ptr<CommandStation>();
}
