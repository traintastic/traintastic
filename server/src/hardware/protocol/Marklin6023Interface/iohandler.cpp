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



#include "iohandler.hpp"
#include "kernel.hpp"
#include "../../../utils/serialport.hpp"

#include <boost/asio/write.hpp>
#include <boost/asio/buffer.hpp>
#include <memory>

namespace Marklin6023 {

IOHandler::IOHandler(Kernel& kernel,
                     boost::asio::io_context& ioContext,
                     boost::asio::io_context::strand& strand,
                     const std::string& device,
                     uint32_t baudrate)
    : m_kernel{kernel}
    , m_strand{strand}
    , m_serialPort{ioContext}
{
    SerialPort::open(m_serialPort, device, baudrate,
                     8, SerialParity::None, SerialStopBits::One,
                     SerialFlowControl::None);
    startRead();
}

IOHandler::~IOHandler()
{
    boost::system::error_code ec;
    m_serialPort.cancel(ec);
    m_serialPort.close(ec);
}

// ---------------------------------------------------------------------------

void IOHandler::sendString(std::string str)
{
    auto buf = std::make_shared<std::string>(std::move(str));

    boost::asio::async_write(
        m_serialPort,
        boost::asio::buffer(*buf),
        m_strand.wrap(
            [this, buf](const boost::system::error_code& ec, std::size_t)
            {
                if(ec && ec != boost::asio::error::operation_aborted)
                    m_kernel.onWriteError(ec);
            }));
}

// ---------------------------------------------------------------------------

void IOHandler::startRead()
{
    if(!m_serialPort.is_open())
        return;

    m_serialPort.async_read_some(
        boost::asio::buffer(m_readBuffer),
        m_strand.wrap(
            [this](const boost::system::error_code& ec, std::size_t bytesRead)
            {
                onRead(ec, bytesRead);
            }));
}

void IOHandler::onRead(const boost::system::error_code& ec, std::size_t bytesRead)
{
    if(ec)
    {
        if(ec != boost::asio::error::operation_aborted)
            m_kernel.onReadError(ec);
        return;
    }

    for(std::size_t i = 0; i < bytesRead; ++i)
    {
        const char c = static_cast<char>(m_readBuffer[i]);
        if(c == '\r' || c == '\n')
        {
            if(!m_lineBuffer.empty())
            {
                m_kernel.receiveLine(std::move(m_lineBuffer));
                m_lineBuffer.clear();
            }
        }
        else
        {
            m_lineBuffer += c;
        }
    }

    startRead();
}

} // namespace Marklin6023
