/**
 * server/src/hardware/protocol/withrottle/iohandler/iohandler.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_WITHROTTLE_IOHANDLER_IOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_WITHROTTLE_IOHANDLER_IOHANDLER_HPP

#include <cstddef>
#include <string_view>

namespace WiThrottle {

class Kernel;

class IOHandler
{
  protected:
    Kernel& m_kernel;

    IOHandler(Kernel& kernel);

#ifndef NDEBUG
    bool isKernelThread() const;
#endif

  public:
    using ClientId = size_t;

    static constexpr ClientId invalidClientId = 0;

    IOHandler(const IOHandler&) = delete;
    IOHandler& operator =(const IOHandler&) = delete;

    virtual ~IOHandler() = default;

    virtual bool hasClients() const = 0;

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual bool sendTo(std::string_view message, ClientId clientId) = 0;
    virtual bool sendToAll(std::string_view message) = 0;

    virtual void disconnect(ClientId clientId) = 0;
};

}

#endif