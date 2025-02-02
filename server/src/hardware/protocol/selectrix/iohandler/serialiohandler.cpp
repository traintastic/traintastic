/**
 * server/src/hardware/protocol/selectrix/iohandler/serialiohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023,2025 Reinder Feenstra
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

#include "serialiohandler.hpp"
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include "../const.hpp"
#include "../kernel.hpp"
#include "../../../../utils/serialport.hpp"
#include "../../../../core/eventloop.hpp"
#include "../../../../log/log.hpp"

namespace {

constexpr uint8_t rautenhausConfig = Selectrix::RautenhausConfig::monitoringOn | Selectrix::RautenhausConfig::feedbackOn;

}

namespace Selectrix {

SerialIOHandler::SerialIOHandler(Kernel& kernel, const std::string& device, uint32_t baudrate, bool useRautenhausCommandFormat)
  : IOHandler(kernel)
  , m_serialPort{kernel.ioContext()}
  , rautenhausCommandFormat{useRautenhausCommandFormat}
{
  SerialPort::open(m_serialPort, device, baudrate, 8, SerialParity::None, SerialStopBits::One, SerialFlowControl::None);
}

SerialIOHandler::~SerialIOHandler()
{
  if(m_serialPort.is_open())
  {
    m_serialPort.close();
  }
}

void SerialIOHandler::start()
{
  read();
  if(rautenhausCommandFormat)
  {
    writeBuffer(Address::selectSXBus, rautenhausConfig);
  }
}

void SerialIOHandler::stop()
{
  m_serialPort.close();
}

bool SerialIOHandler::read(Bus bus, uint8_t address)
{
  assert(address <= Address::max);

  if(!selectBus(bus) || !writeBuffer(address, 0x00))
  {
    return false;
  }

  if(!rautenhausCommandFormat)
  {
    m_readQueue.emplace(BusAddress{bus, address});
  }

  return true;
}

bool SerialIOHandler::write(Bus bus, uint8_t address, uint8_t value)
{
  assert(address <= Address::max);

  if(!selectBus(bus))
  {
    return false;
  }

  return writeBuffer(address |= Address::writeFlag, value);
}

void SerialIOHandler::read()
{
  m_serialPort.async_read_some(boost::asio::buffer(m_readBuffer.data() + m_readBufferOffset, m_readBuffer.size() - m_readBufferOffset),
    [this](const boost::system::error_code& ec, std::size_t bytesTransferred)
    {
      if(!ec)
      {
        processRead(bytesTransferred);
        read();
      }
      else if(ec != boost::asio::error::operation_aborted)
      {
        EventLoop::call(
          [this, ec]()
          {
            Log::log(m_kernel.logId, LogMessage::E2002_SERIAL_READ_FAILED_X, ec);
            m_kernel.error();
          });
      }
    });
}

void SerialIOHandler::processRead(size_t bytesTransferred)
{
  const uint8_t* pos = m_readBuffer.data();
  bytesTransferred += m_readBufferOffset;

  while(bytesTransferred >= (rautenhausCommandFormat ? 2 : 1))
  {
    size_t drop = 0;

    if(rautenhausCommandFormat) // each message is [bus+address][value]
    {
      const auto bus = (pos[0] & 0x80) ? Bus::SX1 : Bus::SX0;
      const uint8_t address = (pos[0] & 0x7F);
      const uint8_t value = pos[1];
      m_kernel.busChanged(bus, address, value);
      pos += 2;
      bytesTransferred -= 2;
    }
    else
    {
      if(!m_readQueue.empty())
      {
        const auto [bus, address] = m_readQueue.front();
        const uint8_t value = *pos;
        m_kernel.busChanged(bus, address, value);
        m_readQueue.pop();
        pos++;
        bytesTransferred--;
      }
      else
      {
        drop += bytesTransferred;
        pos += bytesTransferred;
        bytesTransferred = 0;
      }
    }

    if(drop != 0)
    {
      EventLoop::call(
        [this, drop]()
        {
          Log::log(m_kernel.logId, LogMessage::W2001_RECEIVED_MALFORMED_DATA_DROPPED_X_BYTES, drop);
        });
    }
  }

  if(bytesTransferred != 0)
    memmove(m_readBuffer.data(), pos, bytesTransferred);
  m_readBufferOffset = bytesTransferred;
}

void SerialIOHandler::write()
{
  m_serialPort.async_write_some(boost::asio::buffer(m_writeBuffer.data(), m_writeBufferOffset),
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
          m_writeBufferOffset = 0;
      }
      else if(ec != boost::asio::error::operation_aborted)
      {
        EventLoop::call(
          [this, ec]()
          {
            Log::log(m_kernel.logId, LogMessage::E2001_SERIAL_WRITE_FAILED_X, ec);
            m_kernel.error();
          });
      }
    });
}

bool SerialIOHandler::writeBuffer(uint8_t address, uint8_t value)
{
  if(m_writeBufferOffset + 2 > m_writeBuffer.size())
  {
    return false;
  }

  const bool wasEmpty = m_writeBufferOffset == 0;

  m_writeBuffer[m_writeBufferOffset++] = address;
  m_writeBuffer[m_writeBufferOffset++] = value;

  if(wasEmpty)
  {
    write();
  }

  return true;
}

bool SerialIOHandler::selectBus(Bus bus)
{
  if(m_bus == bus)
  {
    return true;
  }
  m_bus = bus;

  uint8_t value = static_cast<uint8_t>(bus);

  if(rautenhausConfig)
  {
    value &= RautenhausConfig::mask;
    value |= rautenhausConfig;
  }

  return writeBuffer(Address::selectSXBus | Address::writeFlag, value);
}

}
