/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2025 Reinder Feenstra
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

#include "simulatorconnection.hpp"
#include "simulator.hpp"
#include "protocol.hpp"

SimulatorConnection::SimulatorConnection(std::shared_ptr<Simulator> simulator, boost::asio::ip::tcp::socket&& socket,
                                         size_t connId)
  : m_simulator{std::move(simulator)}
  , m_socket{std::move(socket)}
  , m_connectionId(connId)
{
}

void SimulatorConnection::start()
{
  read();
}

void SimulatorConnection::stop()
{
  boost::system::error_code ec;
  m_socket.cancel(ec);
  m_socket.close(ec);
  // ignore errors
}

bool SimulatorConnection::send(const SimulatorProtocol::Message& message)
{
  if(m_writeBufferOffset + message.size > m_writeBuffer.size())
  {
    return false;
  }

  const bool wasEmpty = m_writeBufferOffset == 0;
  memcpy(m_writeBuffer.data() + m_writeBufferOffset, &message, message.size);
  m_writeBufferOffset += message.size;

  if(wasEmpty)
  {
    write();
  }

  return true;
}


void SimulatorConnection::read()
{
  m_socket.async_read_some(boost::asio::buffer(m_readBuffer.data() + m_readBufferOffset, m_readBuffer.size() - m_readBufferOffset),
    [this, keepAlive=shared_from_this()](const boost::system::error_code& ec, std::size_t bytesTransferred)
    {
      if(!ec)
      {
        const std::byte* pos = m_readBuffer.data();
        bytesTransferred += m_readBufferOffset;

        while(bytesTransferred > 1)
        {
          const auto* message = reinterpret_cast<const SimulatorProtocol::Message*>(pos);

          if(bytesTransferred < message->size)
          {
            break;
          }

          m_simulator->receive(*message);
          pos += message->size;
          bytesTransferred -= message->size;
        }

        if(bytesTransferred != 0)
        {
          memmove(m_readBuffer.data(), pos, bytesTransferred);
        }
        m_readBufferOffset = bytesTransferred;

        read();
      }
      else
      {
        close();
      }
    });
}

void SimulatorConnection::write()
{
  if(m_writeBufferOffset == 0) [[unlikely]]
  {
    return;
  }

  m_socket.async_write_some(boost::asio::buffer(m_writeBuffer.data(), m_writeBufferOffset),
    [this, keepAlive=shared_from_this()](const boost::system::error_code& ec, std::size_t bytesTransferred)
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
      else
      {
        close();
      }
    });
}

void SimulatorConnection::close()
{
  m_simulator->removeConnection(shared_from_this());
}
