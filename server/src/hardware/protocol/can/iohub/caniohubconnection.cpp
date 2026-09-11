/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#include "caniohubconnection.hpp"
#include "../../can/canmessage.hpp"
#include "../../can/iohub/caniohub.hpp"
#include "../../../../core/eventloop.hpp"
#include "../../../../log/log.hpp"

namespace CAN {

IOHubConnection::IOHubConnection(std::shared_ptr<IOHub> hub, boost::asio::ip::tcp::socket socket)
  : m_hub{std::move(hub)}
  , m_socket{std::move(socket)}
{
  EventLoop::call(
    [id=m_hub->m_logId, ep=m_socket.remote_endpoint()]()
    {
      Log::log(id, LogMessage::I2007_NEW_HUB_CONNECTION_X_X, ep.address().to_string(), ep.port());
    });
}

IOHubConnection::~IOHubConnection() = default;

void IOHubConnection::start()
{
  read();
}

void IOHubConnection::stop()
{
  boost::system::error_code ec;
  m_socket.cancel(ec);
  m_socket.close(ec);
}

void IOHubConnection::send(const Message& message)
{
  const bool wasEmpty = (m_writeBufferOffset == 0);

  m_writeBufferOffset += serialize(message, std::span(m_writeBuffer).subspan(m_writeBufferOffset));

  if(wasEmpty)
  {
    write();
  }
}

void IOHubConnection::read()
{
  m_socket.async_read_some(boost::asio::buffer(m_readBuffer.data() + m_readBufferOffset, m_readBuffer.size() - m_readBufferOffset),
    [this](const boost::system::error_code& ec, std::size_t bytesTransferred)
    {
      if(!ec)
      {
        m_readBufferOffset += bytesTransferred;
        for(;;)
        {
          CAN::Message canMessage;
          bool hasMessage = false;
          const auto consumed = deserialize(std::span(m_readBuffer.data(), m_readBufferOffset), canMessage, hasMessage);

          if(hasMessage)
          {
            m_hub->receive(canMessage, *this);
          }

          assert(consumed <= m_readBufferOffset);
          if(consumed == m_readBufferOffset)
          {
            m_readBufferOffset = 0;
            break;
          }
          else if(consumed > 0)
          {
            m_readBufferOffset -= consumed;
            memmove(m_readBuffer.data(), m_readBuffer.data() + consumed, m_readBufferOffset);
          }
          else
          {
            break;
          }
        }
        read();
      }
      else if(ec != boost::asio::error::operation_aborted && ec != boost::asio::error::eof)
      {
        EventLoop::call(
          [id=m_hub->m_logId, ec]()
          {
            Log::log(id, LogMessage::E2008_SOCKET_READ_FAILED_X, ec);
          });
        disconnect();
      }
    });
}

void IOHubConnection::write()
{
  assert(m_writeBufferOffset != 0);
  m_socket.async_write_some(boost::asio::buffer(m_writeBuffer.data(), m_writeBufferOffset),
    [this](const boost::system::error_code& ec, std::size_t bytesTransferred)
    {
      if(!ec)
      {
        if(bytesTransferred < m_writeBufferOffset)
        {
          m_writeBufferOffset -= bytesTransferred;
          memmove(m_writeBuffer.data(), m_writeBuffer.data() + bytesTransferred, m_writeBufferOffset);
          write();
        }
        else
        {
          m_writeBufferOffset = 0;
        }
      }
      else if(ec != boost::asio::error::operation_aborted)
      {
        EventLoop::call(
          [id=m_hub->m_logId, ec]()
          {
            Log::log(id, LogMessage::E2007_SOCKET_WRITE_FAILED_X, ec);
          });
        disconnect();
      }
    });
}

void IOHubConnection::disconnect()
{
  boost::asio::post(m_socket.get_executor(),
    [hub=m_hub, conn=shared_from_this()]()
    {
      if(auto it = std::find(hub->m_connections.begin(), hub->m_connections.end(), conn); it != hub->m_connections.end())
      {
        (**it).stop();
        hub->m_connections.erase(it);
      }
    });
}

}
