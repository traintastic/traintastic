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

#include "cbusasciiiohandler.hpp"
#include "../cbuscanmessageascii.hpp"
#include "../cbuskernel.hpp"
#include "../messages/cbusmessage.hpp"
#include "../../../../core/eventloop.hpp"
#include "../../../../log/log.hpp"

namespace CBUS {

ASCIIIOHandler::ASCIIIOHandler(Kernel& kernel)
  : IOHandler(kernel)
{
}

std::error_code ASCIIIOHandler::send(const CAN::Message& canMessage)
{
  const bool wasEmpty = m_writeQueue.empty();

  std::string frame;
  frame.resize(32);
  frame.resize(toAscii(canMessage, frame));
  if(frame.empty()) [[unlikely]]
  {
    return std::make_error_code(std::errc::bad_message);
  }

  m_writeQueue.emplace(std::move(frame));

  if(wasEmpty)
  {
    write();
  }

  return {};
}

void ASCIIIOHandler::processRead(std::size_t bytesTransferred)
{
  std::string_view buffer{m_readBuffer.data(), m_readBufferOffset + bytesTransferred};

  while(!buffer.empty())
  {
    CAN::Message canMessage;
    size_t drop = 0;

    const auto consumed = fromAscii(buffer, canMessage, drop);
    if(drop != 0)
    {
      EventLoop::call(
        [this, drop]()
        {
          Log::log(m_kernel.logId, LogMessage::W2001_RECEIVED_MALFORMED_DATA_DROPPED_X_BYTES, drop);
        });
    }
    if(consumed == 0)
    {
      break; // wait for more data
    }
    if(drop < consumed)
    {
      assert(onReceive);
      onReceive(canMessage);
    }
    buffer.remove_prefix(consumed);
  }

  if(buffer.size() != 0)
  {
    memmove(m_readBuffer.data(), buffer.data(), buffer.size());
  }
  m_readBufferOffset = buffer.size();
}

}
