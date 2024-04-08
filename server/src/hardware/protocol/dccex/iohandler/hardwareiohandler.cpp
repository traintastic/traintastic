/**
 * server/src/hardware/protocol/dccex/iohandler/hardwareiohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
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

namespace DCCEX {

HardwareIOHandler::HardwareIOHandler(Kernel& kernel)
  : IOHandler(kernel)
  , m_readBufferOffset{0}
  , m_writeBufferOffset{0}
{
}

bool HardwareIOHandler::send(std::string_view message)
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

void HardwareIOHandler::processRead(size_t bytesTransferred)
{
  const char* pos = reinterpret_cast<const char*>(m_readBuffer.data());
  bytesTransferred += m_readBufferOffset;

  size_t i = 0;
  while(i < bytesTransferred)
  {
    if(*(pos + i) == '\n')
    {
      m_kernel.receive(std::string_view{pos, i + 1});
      pos += i + 1;
      bytesTransferred -= i + 1;
      i = 0;
    }
    else
      i++;
  }

  if(bytesTransferred != 0)
    memmove(m_readBuffer.data(), pos, bytesTransferred);
  m_readBufferOffset = bytesTransferred;
}

}
