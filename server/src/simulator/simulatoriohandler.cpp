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

#include "simulatoriohandler.hpp"
#include <traintastic/simulator/protocol.hpp>

SimulatorIOHandler::SimulatorIOHandler(boost::asio::io_context& ioContext, std::string hostname, uint16_t port)
  : m_hostname{std::move(hostname)}
  , m_port{port}
  , m_socket{ioContext}
{
}

void SimulatorIOHandler::start()
{
  boost::system::error_code ec;

  m_endpoint.port(m_port);
  m_endpoint.address(boost::asio::ip::make_address(m_hostname, ec));
  if(ec)
  {
    //throw LogMessageException(LogMessage::E2003_MAKE_ADDRESS_FAILED_X, ec);
  }

  m_socket.async_connect(m_endpoint,
    [this](const boost::system::error_code& err)
    {
      if(!err)
      {
        m_socket.set_option(boost::asio::socket_base::linger(true, 0));
        m_socket.set_option(boost::asio::ip::tcp::no_delay(true));

        m_connected = true;

        read();
        write();
      }
      else if(err != boost::asio::error::operation_aborted)
      {
        //EventLoop::call(
        //  [this, err]()
        //  {
            //Log::log(m_kernel.logId, LogMessage::E2005_SOCKET_CONNECT_FAILED_X, err);
            //error();
        //  });
      }
    });
}

void SimulatorIOHandler::stop()
{
}

void SimulatorIOHandler::sendPower(bool on)
{
  const SimulatorProtocol::Power message(on);
  send(message);
}

void SimulatorIOHandler::sendLocomotiveSpeedDirection(DecoderProtocol protocol, uint16_t address, uint8_t speed, Direction direction, bool emergencyStop)
{
  const SimulatorProtocol::LocomotiveSpeedDirection message(address, protocol, speed, direction, emergencyStop);
  send(message);
}

void SimulatorIOHandler::sendAccessorySetState(uint16_t channel, uint16_t address, uint8_t state)
{
  const SimulatorProtocol::AccessorySetState message(channel, address, state);
  send(message);
}

bool SimulatorIOHandler::send(const SimulatorProtocol::Message& message)
{
  if(m_writeBufferOffset + message.size > m_writeBuffer.size())
  {
    return false;
  }

  const bool wasEmpty = m_writeBufferOffset == 0;
  memcpy(m_writeBuffer.data() + m_writeBufferOffset, &message, message.size);
  m_writeBufferOffset += message.size;

  if(wasEmpty && m_connected)
  {
    write();
  }

  return true;
}

void SimulatorIOHandler::receive(const SimulatorProtocol::Message& message)
{
  using namespace SimulatorProtocol;

  switch(message.opCode)
  {
    case OpCode::Power:
      if(onPower)
      {
        onPower(static_cast<const Power&>(message).powerOn);
      }
      break;

    case OpCode::LocomotiveSpeedDirection:
      if(onLocomotiveSpeedDirection)
      {
        const auto& m = static_cast<const LocomotiveSpeedDirection&>(message);
        onLocomotiveSpeedDirection(m.protocol, m.address, m.speed, m.direction, m.emergencyStop != 0);
      }
      break;

    case OpCode::SensorChanged:
      if(onSensorChanged)
      {
        const auto& m = static_cast<const SensorChanged&>(message);
        onSensorChanged(m.channel, m.address, m.value);
      }
      break;

    case OpCode::AccessorySetState:
      if(onAccessorySetState)
      {
        const auto& m = static_cast<const AccessorySetState&>(message);
        onAccessorySetState(m.channel, m.address, m.state);
      }
      break;
  }
}

void SimulatorIOHandler::read()
{
  m_socket.async_read_some(boost::asio::buffer(m_readBuffer.data() + m_readBufferOffset, m_readBuffer.size() - m_readBufferOffset),
    [this](const boost::system::error_code& ec, std::size_t bytesTransferred)
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

          receive(*message);
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
        //EventLoop::call(
        //  [this, ec]()
        //  {
            //Log::log(m_kernel.logId, LogMessage::E2008_SOCKET_READ_FAILED_X, ec);
            //error();
        //  });
      }
    });
}

void SimulatorIOHandler::write()
{
  if(m_writeBufferOffset == 0) /*[[unlikely]]*/
  {
    return;
  }

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
        //EventLoop::call(
        //  [this, ec]()
        //  {
            //Log::log(m_kernel.logId, LogMessage::E2007_SOCKET_WRITE_FAILED_X, ec);
            //error();
        //  });
      }
    });
}
