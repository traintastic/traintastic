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
#include "../cbusgetminorpriority.hpp"
#include "../cbuskernel.hpp"
#include "../messages/cbusmessage.hpp"
#include "../../../../core/eventloop.hpp"
#include "../../../../log/log.hpp"
#include "../../../../utils/tohex.hpp"
#include "../../../../utils/fromchars.hpp"

namespace {

std::string buildFrame(CBUS::MajorPriority majorPriority, CBUS::MinorPriority minorPriority, uint8_t canId, const CBUS::Message& message)
{
  const uint16_t sid =
    (static_cast<uint16_t>(majorPriority) << 14) |
    (static_cast<uint16_t>(minorPriority) << 12) |
    (static_cast<uint16_t>(canId) << 5);

  std::string frame(":S");
  frame.append(toHex(sid));
  frame.append("N");
  frame.append(toHex(&message, message.size()));
  frame.append(";");
  return frame;
}

}

namespace CBUS {

ASCIIIOHandler::ASCIIIOHandler(Kernel& kernel, uint8_t canId)
  : IOHandler(kernel, canId)
{
}

std::error_code ASCIIIOHandler::send(const Message& message)
{
  const bool wasEmpty = m_writeQueue.empty();

  // FIXME: handle priority queueing
  // FIXME: what to do with MajorPriority?
  m_writeQueue.emplace(buildFrame(MajorPriority::Lowest, getMinorPriority(message.opCode), m_canId, message));

  if(wasEmpty)
  {
    write();
  }

  return {};
}

void ASCIIIOHandler::logDropIfNonZeroAndReset(size_t& drop)
{
  if(drop != 0)
  {
    EventLoop::call(
      [this, drop]()
      {
        Log::log(m_kernel.logId, LogMessage::W2001_RECEIVED_MALFORMED_DATA_DROPPED_X_BYTES, drop);
      });
    drop = 0;
  }
}

void ASCIIIOHandler::processRead(std::size_t bytesTransferred)
{
  constexpr size_t maxFrameSize = 24; // :SXXXXNXXXXXXXXXXXXXXXX;

  std::string_view buffer{m_readBuffer.data(), m_readBufferOffset + bytesTransferred};

  size_t drop = 0;

  while(!buffer.empty())
  {
    logDropIfNonZeroAndReset(drop);

    if(auto pos = buffer.find(':'); pos != 0)
    {
      if(pos == std::string_view::npos)
      {
        // no start marker drop all bytes:
        drop += buffer.size();
        buffer = {};
        break;
      }

      // drop bytes before start marker:
      drop += pos;
      buffer.remove_prefix(pos);
    }

    auto end = buffer.find(';');
    if(end == std::string_view::npos)
    {
      // no end marker yet, wait for more data
      break;
    }

    std::string_view frame{buffer.data(), end + 1};

    buffer.remove_prefix(frame.size()); // consume frame

    if(frame.size() > maxFrameSize)
    {
      drop += frame.size();
    }
    else if(frame[1] == 'S' && frame[6] == 'N') // CBUS only uses standard non RTR CAN frames
    {
      uint16_t sid;
      if(fromChars(frame.substr(2, sizeof(sid) * 2), sid, 16).ec != std::errc())
      {
        continue; // error reading, ignore frame
      }

      const auto dataLength = (frame.size() - 7) / 2;
      std::array<uint8_t, 8> data;
      std::errc ec = std::errc();
      for(size_t i = 0; i < dataLength; ++i)
      {
        ec = fromChars(frame.substr(7 + i * 2, 2), data[i], 16).ec;
        if(ec != std::errc())
        {
          break;
        }
      }
      if(ec != std::errc())
      {
        continue; // error reading, ignore frame
      }

      m_kernel.receive((sid >> 5) & 0x7F, *reinterpret_cast<const Message*>(data.data()));
    }
  }

  logDropIfNonZeroAndReset(drop);

  if(buffer.size() != 0)
  {
    memmove(m_readBuffer.data(), buffer.data(), buffer.size());
  }
  m_readBufferOffset = buffer.size();
}

}
