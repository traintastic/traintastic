/**
 * server/src/hardware/protocol/marklincan/iohandler/networkiohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#include "networkiohandler.hpp"
#include "../../../../utils/endian.hpp"

namespace MarklinCAN {

void NetworkIOHandler::start()
{
  read();
}

bool NetworkIOHandler::send(const Message& message)
{
  if(m_writeBufferOffset + sizeof(NetworkMessage) > m_writeBuffer.size())
    return false;

  const bool wasEmpty = m_writeBufferOffset == 0;
  toNetworkMessage(message, *reinterpret_cast<NetworkMessage*>(m_writeBuffer.data() + m_writeBufferOffset));
  m_writeBufferOffset += sizeof(NetworkMessage);

  if(wasEmpty)
    write();

  return true;
}

Message NetworkIOHandler::toMessage(const NetworkMessage& networkMessage)
{
  Message message;
  message.id = be_to_host(networkMessage.idBE);
  message.dlc = networkMessage.dlc;
  memcpy(message.data, networkMessage.data, std::min(sizeof(message.data), sizeof(networkMessage.data)));
  return message;
}

void NetworkIOHandler::toNetworkMessage(const Message& message, NetworkMessage& networkMessage)
{
  networkMessage.idBE = host_to_be(message.id);
  networkMessage.dlc = message.dlc;
  memcpy(networkMessage.data, message.data, std::min(sizeof(networkMessage.data), sizeof(message.data)));
}

}
