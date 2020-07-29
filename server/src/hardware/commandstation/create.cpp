/**
 * server/src/hardware/commandstation/create.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

#include "create.hpp"
#include "li10x.hpp"
#include "loconetserial.hpp"
#ifndef DISABLE_USB_XPRESSNET_INTERFACE
  #include "usbxpressnetinterface.hpp"
#endif
#include "z21.hpp"

namespace Hardware::CommandStation {

const std::vector<std::string_view>& classList()
{
  static std::vector<std::string_view> list({
    LI10x::classId,
    LocoNetSerial::classId,
#ifndef DISABLE_USB_XPRESSNET_INTERFACE
    USBXpressNetInterface::classId,
#endif
    Z21::classId,
  });
  return list;
}

std::shared_ptr<CommandStation> create(const std::weak_ptr<World>& world, std::string_view classId, std::string_view id)
{
  if(classId == LI10x::classId)
    return LI10x::create(world, id);
  else if(classId == LocoNetSerial::classId)
    return LocoNetSerial::create(world, id);
#ifndef DISABLE_USB_XPRESSNET_INTERFACE
  else if(classId == USBXpressNetInterface::classId)
    return USBXpressNetInterface::create(world, id);
#endif
  else if(classId == Z21::classId)
    return Z21::create(world, id);
  else
    return std::shared_ptr<CommandStation>();
}

}
