/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2023-2026 Reinder Feenstra
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

#include "socketcaniohandler.hpp"
#include "../kernel.hpp"
#include "../messages.hpp"

namespace MarklinCAN {

SocketCANIOHandler::SocketCANIOHandler(Kernel& kernel, const std::string& interface)
  : IOHandler(kernel)
  , m_socketCAN{kernel.ioContext(), interface, m_kernel.logId,
      [this](const CAN::SocketCANIOHandler::Frame& frame)
      {
        Message message;
        message.id = frame.can_id & CAN_EFF_MASK;
        message.dlc = frame.can_dlc;
        std::memcpy(message.data, frame.data, message.dlc);
        m_kernel.receive(message);
      },
      std::bind(&Kernel::error, &m_kernel)}
{
}

void SocketCANIOHandler::start()
{
  m_socketCAN.start();
  m_kernel.started();
}

void SocketCANIOHandler::stop()
{
  m_socketCAN.stop();
}

bool SocketCANIOHandler::send(const Message& message)
{
  CAN::SocketCANIOHandler::Frame frame;
  frame.can_id = CAN_EFF_FLAG | (message.id & CAN_EFF_MASK);
  frame.can_dlc = message.dlc;
  std::memcpy(frame.data, message.data, message.dlc);
  return m_socketCAN.send(frame);
}

}
