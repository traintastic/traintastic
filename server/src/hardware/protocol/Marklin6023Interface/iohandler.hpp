/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Kamil Kasprzak
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



#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLIN6023INTERFACE_IOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLIN6023INTERFACE_IOHANDLER_HPP

#include <string>
#include <array>
#include <vector>
#include <cstdint>
#include <boost/asio/io_context.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/asio/strand.hpp>
#include <boost/system/error_code.hpp>

namespace Marklin6023 {

class Kernel;

class IOHandler final
{
public:
    IOHandler(Kernel& kernel,
              boost::asio::io_context& ioContext,
              boost::asio::io_context::strand& strand,
              const std::string& device,
              uint32_t baudrate);

    ~IOHandler();

    /** Send a complete command string (including CR). Must be on the strand. */
    void sendString(std::string str);

private:
    static constexpr std::size_t kReadBufferSize = 256;

    Kernel&                          m_kernel;
    boost::asio::io_context::strand& m_strand;
    boost::asio::serial_port         m_serialPort;

    std::array<uint8_t, kReadBufferSize> m_readBuffer;
    std::string                          m_lineBuffer;

    void startRead();
    void onRead(const boost::system::error_code& ec, std::size_t bytesRead);
};

} // namespace Marklin6023

#endif
