/**
 * server/src/hardware/protocol/loconet/iohandler/lbserveriohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023-2024 Reinder Feenstra
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

#include "lbserveriohandler.hpp"
#include <boost/asio/write.hpp>
#include "../kernel.hpp"
#include "../messages.hpp"
#include "../../../../core/eventloop.hpp"
#include "../../../../log/log.hpp"
#include "../../../../utils/startswith.hpp"
#include "../../../../utils/tohex.hpp"

static std::string formatSend(const LocoNet::Message& message)
{
  std::string send{"SEND"};
  send.reserve(5 + 3 * message.size());
  for(uint8_t i = 0; i < message.size(); i++)
    send.append(" ").append(toHex(*(reinterpret_cast<const uint8_t*>(&message) + i)));
  send.append("\n");
  return send;
}

static constexpr bool isHexDigit(char c)
{
  return
    (c >= '0' && c <= '9') ||
    (c >= 'A' && c <= 'F') ||
    (c >= 'a' && c <= 'f');
}

static constexpr uint8_t hexDigitValue(char c)
{
  if(c >= '0' && c <= '9')
    return static_cast<uint8_t>(c - '0');
  if(c >= 'A' && c <= 'F')
    return static_cast<uint8_t>(c - 'A' + 10);
  if(c >= 'a' && c <= 'f')
    return static_cast<uint8_t>(c - 'a' + 10);
  return 0xFF;
}

static std::vector<std::byte> readHexBytes(std::string_view text)
{
  std::vector<std::byte> bytes;
  size_t i = 0;
  while(i < text.size() - 1)
  {
    if(isHexDigit(text[i]) && isHexDigit(text[i + 1]))
    {
      bytes.emplace_back(static_cast<std::byte>((hexDigitValue(text[i]) * 16) | hexDigitValue(text[i + 1])));
      i += 2;
    }
    else
      i++;
  }
  return bytes;
}

namespace LocoNet {

LBServerIOHandler::LBServerIOHandler(Kernel& kernel, std::string hostname, uint16_t port)
  : TCPIOHandler(kernel, std::move(hostname), port)
  , m_readBufferOffset{0}
{
}

bool LBServerIOHandler::send(const Message& message)
{
  const bool wasEmpty = m_writeQueue.empty();
  m_writeQueue.emplace(formatSend(message));

  if(wasEmpty && connected())
    write();

  return true;
}

void LBServerIOHandler::read()
{
  m_socket.async_read_some(boost::asio::buffer(m_readBuffer.data() + m_readBufferOffset, m_readBuffer.size() - m_readBufferOffset),
    [this](const boost::system::error_code& ec, std::size_t bytesTransferred)
    {
      if(!ec)
      {
        const char* pos = m_readBuffer.data();
        bytesTransferred += m_readBufferOffset;
        const char* end = pos + bytesTransferred;

        while(bytesTransferred > 0)
        {
          const char* eol = pos;
          while(*eol != '\n' && *eol != '\r' && eol < end)
            eol++;

          if(eol == end)
            break; // no newline found, wait for more data

          std::string_view line{pos, static_cast<size_t>(eol - pos)};

          if(startsWith(line, "RECEIVE "))
          {
            std::vector<std::byte> bytes = readHexBytes(line.substr(8));
            const auto* message = reinterpret_cast<const Message*>(bytes.data());
            if(isValid(*message))
              m_kernel.receive(*message);
          }
          else if(startsWith(line, "SENT OK"))
          {
            m_writeQueue.pop();
            if(!m_writeQueue.empty())
              write();
          }
          else if(startsWith(line, "VERSION "))
          {
            m_version = line.substr(7);
          }

          pos = eol + 1; // Skip the newline character
          if (pos < end && (*pos == '\n' || *pos == '\r') && *pos != *(pos - 1)) {
            pos++; // Skip the second part of CRLF or LFCR
          }
          bytesTransferred = end - pos;
        }

        if(bytesTransferred != 0)
          memmove(m_readBuffer.data(), pos, bytesTransferred);
        m_readBufferOffset = bytesTransferred;

        read();
      }
      else
      {
        EventLoop::call(
          [this, ec]()
          {
            Log::log(m_kernel.logId, LogMessage::E2008_SOCKET_READ_FAILED_X, ec);
            error();
          });
      }
    });
}

void LBServerIOHandler::write()
{
  if(m_writeQueue.empty()) /*[[unlikely]]*/
  {
    return;
  }

  const std::string& message = m_writeQueue.front();
  boost::asio::async_write(m_socket, boost::asio::buffer(message.data(), message.size()),
    [this](const boost::system::error_code& ec, std::size_t /*bytesTransferred*/)
    {
      if(ec && ec != boost::asio::error::operation_aborted)
      {
        EventLoop::call(
          [this, ec]()
          {
            Log::log(m_kernel.logId, LogMessage::E2007_SOCKET_WRITE_FAILED_X, ec);
            error();
          });
      }
    });
}

}
