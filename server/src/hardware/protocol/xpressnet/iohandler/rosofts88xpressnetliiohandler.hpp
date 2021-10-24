/**
 * server/src/hardware/protocol/xpressnet/iohandler/rosofts88xpressnetliiohandler.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_XPRESSNET_IOHANDLER_ROSOFTS88XPRESSNETLIIOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_XPRESSNET_IOHANDLER_ROSOFTS88XPRESSNETLIIOHANDLER_HPP

#include "serialiohandler.hpp"

namespace XpressNet {

class RoSoftS88XPressNetLIIOHandler final : public SerialIOHandler
{
  private:
    const uint8_t m_s88StartAddress;
    const uint8_t m_s88ModuleCount;

  public:
    RoSoftS88XPressNetLIIOHandler(Kernel& kernel, const std::string& device, uint32_t baudrate, SerialFlowControl flowControl, uint8_t s88StartAddress, uint8_t s88ModuleCount);

    void start() final;
};

}

#endif
