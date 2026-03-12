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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLIN6050_IOHANDLER_SERIALIOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLIN6050_IOHANDLER_SERIALIOHANDLER_HPP

#include "iohandler.hpp"

#include <array>
#include <string>
#include <boost/asio/io_context.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/asio/strand.hpp>
#include <boost/system/error_code.hpp>

namespace Marklin6050 {

class SerialIOHandler final : public IOHandler
{
public:
  SerialIOHandler(Kernel& kernel,
                  boost::asio::io_context& ioContext,
                  boost::asio::io_context::strand& strand,
                  std::string device,
                  uint32_t baudrate);

  ~SerialIOHandler() final;

  void start() final;
  void stop() final;
  void send(std::initializer_list<uint8_t> bytes) final;

private:
  static constexpr std::size_t kReadBufferSize = 256;

  boost::asio::io_context::strand&     m_strand;
  boost::asio::serial_port             m_serialPort;
  const std::string                    m_device;
  const uint32_t                       m_baudrate;
  std::array<uint8_t, kReadBufferSize> m_readBuffer;

  void startRead();
  void onRead(const boost::system::error_code& ec, std::size_t bytesRead);
};

} // namespace Marklin6050

#endif
