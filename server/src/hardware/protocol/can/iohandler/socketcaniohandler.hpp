/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2023-2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CAN_IOHANDLER_SOCKETCANIOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CAN_IOHANDLER_SOCKETCANIOHANDLER_HPP

#include <string>
#include <functional>
#include <span>
#include <linux/can.h>
#include <boost/asio/io_context.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>

namespace CAN {

class SocketCANIOHandler
{
public:
  using Frame = struct can_frame;
  using Filter = struct can_filter;
  using OnReceive = std::function<void(const Frame& frame)>;
  using OnError = std::function<void()>;

  SocketCANIOHandler(boost::asio::io_context& ioContext, const std::string& interface, std::string logId, OnReceive onReceive, OnError onError, std::span<Filter> filter = {});

  void start();
  void stop();

  bool send(const Frame& frame);

private:
  static constexpr size_t frameSize = sizeof(Frame);

  boost::asio::posix::stream_descriptor m_stream;
  std::array<Frame, 32> m_readBuffer;
  size_t m_readBufferOffset = 0;
  std::array<Frame, 32> m_writeBuffer;
  size_t m_writeBufferOffset = 0;
  const std::string m_logId;
  OnReceive m_onReceive;
  OnError m_onError;

  void read();
  void write();
};

}

#endif
