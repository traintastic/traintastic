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

#include "dinamoserialiohandler.hpp"
#include <boost/asio/write.hpp>
#include "../dinamoerror.hpp"
#include "../dinamokernel.hpp"
#include "../../../../core/eventloop.hpp"
#include "../../../../log/log.hpp"
#include "../../../../utils/bit.hpp"
#include "../../../../utils/serialport.hpp"

namespace {

using namespace std::chrono_literals;

constexpr auto responseTimeout = 200ms;

constexpr uint8_t txRetryLimit = 10;

constexpr size_t headerSize = 1;
constexpr size_t checksumSize = 1;
constexpr size_t minDatagramSize = headerSize + checksumSize;

constexpr uint8_t headerX012Mask = 0x07;
constexpr uint8_t headerX34Mask = 0x30;

constexpr uint8_t headerJumboBit = 3;
constexpr uint8_t headerHoldBit = 4;
constexpr uint8_t headerFaultBit = 5;
constexpr uint8_t headerToggleBit = 6;

constexpr uint8_t dataByteBit = 7;
constexpr uint8_t dataByteMask = 1 << dataByteBit;

constexpr uint8_t calcChecksum(std::span<const uint8_t> messageWithoutChecksum) noexcept
{
  uint8_t checksum = 0;
  for(auto byte : messageWithoutChecksum)
  {
    checksum += byte & 0x7F;
    checksum &= 0x7F;
  }
  return 0x80 | (~checksum + 1);
}

constexpr bool isChecksumValid(std::span<const uint8_t> message) noexcept
{
  return calcChecksum(message.subspan(0, message.size() - 1)) == *message.rbegin();
}

constexpr bool isJumboDatagram(uint8_t header) noexcept
{
  return !getBit<headerJumboBit>(header); // 0 = jumbo, 1 = normal
}

constexpr uint8_t getPayloadSize(uint8_t header) noexcept
{
  if(isJumboDatagram(header))
  {
    return 8 + (header & headerX012Mask) + ((header & headerX34Mask) >> 1);
  }
  return (header & headerX012Mask); // normal datagram
}

constexpr uint8_t getDatagramSize(uint8_t header) noexcept
{
  return headerSize + getPayloadSize(header) + checksumSize;
}

}

namespace Dinamo {

SerialIOHandler::SerialIOHandler(Kernel& kernel, const std::string& device)
  : IOHandler(kernel)
  , m_serialPort{m_kernel.ioContext()}
{
  SerialPort::open(m_serialPort, device, 19'200, 8, SerialParity::Odd, SerialStopBits::One, SerialFlowControl::None);
}

SerialIOHandler::~SerialIOHandler()
{
  if(m_serialPort.is_open())
  {
    boost::system::error_code ec;
    m_serialPort.close(ec);
    // ignore the error
  }
}

void SerialIOHandler::stop()
{
  m_serialPort.close();
}

std::error_code SerialIOHandler::send(std::span<const uint8_t> message, bool hold, bool fault)
{
  if(message.size() > maxMessageSize) [[unlikely]]
  {
    assert(false);
    return Error::messageTooLarge();
  }

  // cache last sent values, in case we need to send a Null datagram:
  m_txHold = hold;
  m_txFault = fault;

  const auto datagramSize = headerSize + message.size() + checksumSize;

  if(m_txBufferOffset + datagramSize > m_txBuffer.size())
  {
    return Error::bufferFull();
  }

  auto* const begin = m_txBuffer.data() + m_txBufferOffset;
  auto* const end = begin + datagramSize;

  // header:
  auto& header = *begin;
  if(message.size() >= 8) // jumbo: [0 T X4 X3 J X2 X1 X0]
  {
    header = static_cast<uint8_t>(message.size() - 8) & headerX012Mask;
    clearBit<headerJumboBit>(header); // jumbo (0)
    header |= (static_cast<uint8_t>(message.size() - 8) << 1) & headerX34Mask;
  }
  else // normal: [0 T F H J X2 X1 X0]
  {
    header = static_cast<uint8_t>(message.size()) & headerX012Mask;
    setBit<headerJumboBit>(header); // normal (1)
    setBit<headerHoldBit>(header, hold);
    setBit<headerFaultBit>(header, fault);
  }
  setBit<headerToggleBit>(header, m_txToggle);
  m_txToggle = !m_txToggle;

  // payload: [1 D6 D5 D4 D3 D2 D1 D0]
  std::memcpy(begin + headerSize, message.data(), message.size());
  for(auto* p = begin + headerSize; p < end - checksumSize; ++p)
  {
    setBit<dataByteBit>(*p, true);
  }

  // checksum: [1 CS6 CS5 CS4 CS3 CS2 CS1 CS0]
  *(end - checksumSize) = calcChecksum({begin, end - checksumSize});

  m_txBufferOffset += datagramSize;

  if(m_txIdle)
  {
    write();
  }

  return {};
}

void SerialIOHandler::read()
{
  m_serialPort.async_read_some(boost::asio::buffer(m_rxBuffer.data() + m_rxBufferOffset, m_rxBuffer.size() - m_rxBufferOffset),
    [this](const boost::system::error_code& ec, std::size_t bytesTransferred)
    {
      if(!ec)
      {
        std::optional<std::span<uint8_t>> message;
        bool hold = m_rxHold;
        bool fault = m_rxFault;

        size_t drop = 0;
        auto* pos = m_rxBuffer.data();
        bytesTransferred += m_rxBufferOffset;

        while(bytesTransferred >= minDatagramSize)
        {
          while((*pos & dataByteMask) != 0) // drop data bytes
          {
            drop++;
            pos++;
            bytesTransferred--;
          }

          const auto header = *pos;
          const auto datagramSize = getDatagramSize(header);

          if(datagramSize <= bytesTransferred)
          {
            if(isChecksumValid({pos, pos + datagramSize}) && getBit<headerToggleBit>(header) == getBit<headerToggleBit>(m_txBuffer[0]))
            {
              message = std::span<uint8_t>{pos + headerSize, pos + datagramSize - checksumSize};

              if(!isJumboDatagram(header)) // only available if normal datagram
              {
                fault = getBit<headerFaultBit>(header);
                hold = getBit<headerHoldBit>(header);
              }

              drop += bytesTransferred - datagramSize; // drop remaining bytes (should be zero)
              m_rxBufferOffset = 0;
              break;
            }
            else // drop header byte
            {
              drop++;
              pos++;
              bytesTransferred--;
            }
          }
          else
            break;
        }

        if(drop != 0)
        {
          EventLoop::call(
            [this, drop]()
            {
              Log::log(m_kernel.logId, LogMessage::W2001_RECEIVED_MALFORMED_DATA_DROPPED_X_BYTES, drop);
            });
        }

        if(message) // we got a response
        {
          m_timer.cancel();
          m_txRetries = 0;

          const auto sentDatagramSize = getDatagramSize(m_txBuffer[0]);
          if(m_txBufferOffset > sentDatagramSize)
          {
            memmove(m_txBuffer.data(), m_txBuffer.data() + sentDatagramSize, m_txBufferOffset - sentDatagramSize);
          }
          m_txBufferOffset -= sentDatagramSize;

          if(!message->empty() || hold != m_rxHold || fault != m_rxFault) // only if not NULL or hold/fault is changed
          {
            // clear databyte bits:
            for(auto& by : *message)
            {
              clearBit<dataByteBit>(by);
            }

            m_kernel.receive(*message, hold, fault);
          }
          m_rxHold = hold;
          m_rxFault = fault;

          m_txIdle = true;
          if(m_txBufferOffset != 0)
          {
            write();
          }
          else
          {
            startIdleTimeoutTimer();
          }
        }
        else // keep reading
        {
          if(bytesTransferred != 0)
          {
            memmove(m_rxBuffer.data(), pos, bytesTransferred);
          }
          m_rxBufferOffset = bytesTransferred;

          read();
        }
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

void SerialIOHandler::write()
{
  assert(m_txIdle);

  m_txIdle = false;

  boost::asio::async_write(m_serialPort, boost::asio::buffer(m_txBuffer.data(), getDatagramSize(m_txBuffer[0])),
    [this](const boost::system::error_code& ec, std::size_t /*bytesTransferred*/)
    {
      if(!ec)
      {
        startResponseTimeoutTimer();
        read();
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

void SerialIOHandler::startResponseTimeoutTimer()
{
  m_timer.cancel();
  m_timer.expires_from_now(responseTimeout);
  m_timer.async_wait(
    [this](const boost::system::error_code& ec)
    {
      if(!ec) // no response
      {
        if(++m_txRetries == txRetryLimit)
        {
          EventLoop::call(
            [this]()
            {
              Log::log(m_kernel.logId, LogMessage::E2030_COMMUNICATION_LOST_NO_RESPONSE_WITHIN_X_MS, responseTimeout.count());
              m_kernel.error();
            });
        }
        else
        {
          EventLoop::call(
            [this]()
            {
              Log::log(m_kernel.logId, LogMessage::W2028_NO_RESPONSE_WITHIN_X_MS_RESENDING_LAST_MESSAGE, responseTimeout.count());
            });
        }

        m_txIdle = true;
        write(); // retry
      }
    });
}

}
