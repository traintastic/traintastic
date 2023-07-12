/**
 * server/src/hardware/protocol/marklincan/iohandler/socketcaniohandler.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLINCAN_IOHANDLER_SOCKETCANIOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLINCAN_IOHANDLER_SOCKETCANIOHANDLER_HPP

#include "iohandler.hpp"
#include <string>
#include <linux/can.h>
#include <boost/asio/posix/stream_descriptor.hpp>

namespace MarklinCAN {

class SocketCANIOHandler : public IOHandler
{
  private:
    static constexpr size_t frameSize = sizeof(struct can_frame);

    boost::asio::posix::stream_descriptor m_stream;
    std::array<struct can_frame, 32> m_readBuffer;
    size_t m_readBufferOffset = 0;
    std::array<struct can_frame, 32> m_writeBuffer;
    size_t m_writeBufferOffset = 0;

    void read();
    void write();

  public:
    SocketCANIOHandler(Kernel& kernel, const std::string& interface);

    void start() final;
    void stop() final;

    bool send(const Message& message) final;
};

}

#endif
