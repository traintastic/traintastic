/**
 * server/src/hardware/protocol/xpressnet/iohandler/hardwareiohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2023 Reinder Feenstra
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

#include "hardwareiohandler.hpp"
#include "../kernel.hpp"
#include "../messages.hpp"
#include "../../../../core/eventloop.hpp"
#include "../../../../log/log.hpp"

namespace XpressNet {

HardwareIOHandler::HardwareIOHandler(Kernel& kernel)
  : IOHandler{kernel}
  , m_readBufferOffset{0}
  , m_writeBufferOffset{0}
  , m_extraHeader{false}
{
}

bool HardwareIOHandler::send(const Message& message)
{
  if(m_writeBufferOffset + message.size() > m_writeBuffer.size())
    return false;

  const bool wasEmpty = m_writeBufferOffset == 0;

  if(m_extraHeader)
  {
    m_writeBuffer[m_writeBufferOffset++] = std::byte{0xFF};
    m_writeBuffer[m_writeBufferOffset++] = std::byte{0xFE};
  }

  memcpy(m_writeBuffer.data() + m_writeBufferOffset, &message, message.size());
  m_writeBufferOffset += message.size();

  if(wasEmpty)
    write();

  return true;
}

void HardwareIOHandler::processRead(size_t bytesTransferred)
{
  const std::byte* pos = m_readBuffer.data();
  bytesTransferred += m_readBufferOffset;

  while(bytesTransferred > (m_extraHeader ? 3 : 1))
  {
    const Message* message = nullptr;
    size_t drop = 0;

    if(m_extraHeader) // each message is prepended by [FF FE] or [FF FD]
    {
      while(drop < bytesTransferred - 2)
      {
        message = reinterpret_cast<const Message*>(pos + 2);
        if(pos[0] == std::byte{0xFF} &&
            (pos[1] == std::byte{0xFE} || pos[1] == std::byte{0xFD}) &&
            message->size() <= bytesTransferred &&
            isChecksumValid(*message))
        {
          pos += 2;
          bytesTransferred -= 2;
          break;
        }

        drop++;
        pos++;
        bytesTransferred--;
      }
    }
    else
    {
      while(drop < bytesTransferred)
      {
        message = reinterpret_cast<const Message*>(pos);
        if(message->size() <= bytesTransferred && isChecksumValid(*message))
          break;

        drop++;
        pos++;
        bytesTransferred--;
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

    assert(message);
    if(message->size() <= bytesTransferred)
    {
      m_kernel.receive(*message);
      pos += message->size();
      bytesTransferred -= message->size();
    }
    else
      break;
  }

  if(bytesTransferred != 0)
    memmove(m_readBuffer.data(), pos, bytesTransferred);
  m_readBufferOffset = bytesTransferred;
}

}
