/**
 * server/src/hardware/protocol/traintasticdiy/iohandler/hardwareiohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2023 Reinder Feenstra
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
#include "../../../../utils/tohex.hpp"

namespace TraintasticDIY {

HardwareIOHandler::HardwareIOHandler(Kernel& kernel)
  : IOHandler{kernel}
  , m_readBufferOffset{0}
  , m_writeBufferOffset{0}
{
}

bool HardwareIOHandler::send(const Message& message)
{
  if(m_writeBufferOffset + message.size() > m_writeBuffer.size())
    return false;

  const bool wasEmpty = m_writeBufferOffset == 0;

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

  while(bytesTransferred > 1)
  {
    const Message* message = nullptr;
    size_t drop = 0;

    while(drop < bytesTransferred)
    {
      message = reinterpret_cast<const Message*>(pos);
      if(message->size() > bytesTransferred || isChecksumValid(*message))
        break;

      drop++;
      pos++;
      bytesTransferred--;
    }

    if(drop != 0)
    {
      EventLoop::call(
        [this, drop, bytes=toHex(pos - drop, drop, true)]()
        {
          Log::log(m_kernel.logId, LogMessage::W2003_RECEIVED_MALFORMED_DATA_DROPPED_X_BYTES_X, drop, bytes);
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
