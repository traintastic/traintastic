/**
 * server/src/hardware/protocol/ecos/iohandler/iohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#include "iohandler.hpp"
#include "../kernel.hpp"

namespace ECoS {

IOHandler::IOHandler(Kernel& kernel)
  : m_kernel{kernel}
  , m_readBufferOffset{0}
  , m_readPos{0}
  , m_writeBufferOffset{0}
{
}

bool IOHandler::send(std::string_view message)
{
  if(m_writeBufferOffset + message.size() > m_writeBuffer.size())
    return false;

  const bool wasEmpty = m_writeBufferOffset == 0;
  memcpy(m_writeBuffer.data() + m_writeBufferOffset, message.data(), message.size());
  m_writeBufferOffset += message.size();

  if(wasEmpty)
    write();

  return true;
}

void IOHandler::processRead(size_t bytesTransferred)
{
  static constexpr std::string_view typeReply{"REPLY"};
  static constexpr std::string_view typeEvent{"EVENT"};
  static constexpr size_t typeLength = 5;

  std::string_view buffer{m_readBuffer.data(), m_readBufferOffset + bytesTransferred};

  //! @todo this can be a bit optimized by remembering the "state" when a message in not yet complete.
  while(m_readPos != buffer.size())
  {
    m_readPos = buffer.find('<', m_readPos);
    if(m_readPos != std::string_view::npos)
    {
      if((buffer.size() - m_readPos) >= typeLength)
      {
        std::string_view type{m_readBuffer.data() + m_readPos + 1, typeLength};
        if(type == typeReply || type == typeEvent)
        {
          size_t pos = buffer.find(std::string_view{"<END"}, m_readPos);
          if(pos != std::string_view::npos)
          {
            size_t end = buffer.find('>', pos);
            if(end != buffer.size())
            {
              m_kernel.receive(std::string_view{m_readBuffer.data() + m_readPos, end - m_readPos + 1});
              m_readPos = end + 1;
            }
            else
              break;
          }
          else
            break;
        }
      }
      else
        break;
    }
    else
      m_readPos = buffer.size();
  }

  if(m_readPos > 0)
  {
    assert(m_readPos <= buffer.size());
    m_readBufferOffset = buffer.size() - m_readPos;
    if(m_readBufferOffset > 0)
      memmove(m_readBuffer.data(), m_readBuffer.data() + m_readPos, m_readBufferOffset);
    m_readPos = 0;
  }
}

}
