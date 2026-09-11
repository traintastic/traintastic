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

#include "socketcaniohandler.hpp"
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can/raw.h>
#include "../../../../core/eventloop.hpp"
#include "../../../../log/log.hpp"
#include "../../../../log/logmessageexception.hpp"

namespace CAN {

SocketCANIOHandler::SocketCANIOHandler(boost::asio::io_context& ioContext, const std::string& interface, std::string logId, OnReceive onReceive, OnError onError, std::span<Filter> filter)
  : m_stream{ioContext}
  , m_logId{std::move(logId)}
  , m_onReceive{std::move(onReceive)}
  , m_onError{std::move(onError)}
{
  assert(m_onReceive);
  assert(m_onError);

  int fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if(fd < 0)
  {
    throw LogMessageException(LogMessage::E2022_SOCKET_CREATE_FAILED_X, std::string_view(strerror(errno)));
  }

  struct ifreq ifr;
  std::strncpy(ifr.ifr_name, interface.c_str(), IFNAMSIZ - 1);
  ifr.ifr_name[IFNAMSIZ - 1] = '\0';
  if(ioctl(fd, SIOCGIFINDEX, &ifr) < 0)
  {
    close(fd);
    throw LogMessageException(LogMessage::E2023_SOCKET_IOCTL_FAILED_X, std::string_view(strerror(errno)));
  }

  struct sockaddr_can addr;
  addr.can_family = AF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;
  if(bind(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0)
  {
    close(fd);
    throw LogMessageException(LogMessage::E2006_SOCKET_BIND_FAILED_X, std::string_view(strerror(errno)));
  }

  if(!filter.empty() && setsockopt(fd, SOL_CAN_RAW, CAN_RAW_FILTER, filter.data(), sizeof(Filter) * filter.size()) < 0)
  {
    close(fd);
    throw LogMessageException(LogMessage::E2028_SOCKET_SETSOCKOPT_FAILED_X, std::string_view(strerror(errno)));
  }

  m_stream.assign(fd);
}

void SocketCANIOHandler::start()
{
  read();
}

void SocketCANIOHandler::stop()
{
  m_stream.cancel();
  m_stream.close();
}

bool SocketCANIOHandler::send(const Frame& frame)
{
  if(m_writeBufferOffset + 1 > m_writeBuffer.size())
  {
    return false;
  }

  const bool wasEmpty = m_writeBufferOffset == 0;

  std::memcpy(&m_writeBuffer[m_writeBufferOffset], &frame, sizeof(frame));

  m_writeBufferOffset++;

  if(wasEmpty)
  {
    write();
  }

  return true;
}

void SocketCANIOHandler::read()
{
  m_stream.async_read_some(boost::asio::buffer(m_readBuffer.data() + m_readBufferOffset * frameSize, (m_readBuffer.size() - m_readBufferOffset) * frameSize),
    [this](const boost::system::error_code& ec, std::size_t bytesTransferred)
    {
      if(!ec)
      {
        auto framesTransferred = bytesTransferred / frameSize;
        const auto* frame = reinterpret_cast<const Frame*>(m_readBuffer.data());
        framesTransferred += m_readBufferOffset;

        while(framesTransferred > 0)
        {
          m_onReceive(*frame);
          frame++;
          framesTransferred--;
        }

        if(framesTransferred != 0) [[unlikely]]
        {
          memmove(m_readBuffer.data(), frame, framesTransferred * frameSize);
        }
        m_readBufferOffset = framesTransferred;

        read();
      }
      else if(ec != boost::asio::error::operation_aborted)
      {
        EventLoop::call(
          [this, ec]()
          {
            Log::log(m_logId, LogMessage::E2008_SOCKET_READ_FAILED_X, ec);
            m_onError();
          });
      }
    });
}

void SocketCANIOHandler::write()
{
  m_stream.async_write_some(boost::asio::buffer(m_writeBuffer.data(), m_writeBufferOffset * frameSize),
    [this](const boost::system::error_code& ec, std::size_t bytesTransferred)
    {
      if(!ec)
      {
        const auto framesTransferred = bytesTransferred / frameSize;
        if(framesTransferred < m_writeBufferOffset)
        {
          m_writeBufferOffset -= framesTransferred;
          memmove(m_writeBuffer.data(), m_writeBuffer.data() + framesTransferred * frameSize, m_writeBufferOffset);
          write();
        }
        else
          m_writeBufferOffset = 0;
      }
      else if(ec != boost::asio::error::operation_aborted)
      {
        EventLoop::call(
          [this, ec]()
          {
            Log::log(m_logId, LogMessage::E2007_SOCKET_WRITE_FAILED_X, ec);
            m_onError();
          });
      }
    });
}

}
