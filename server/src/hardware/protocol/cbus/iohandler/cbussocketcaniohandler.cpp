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

#include "cbussocketcaniohandler.hpp"
#include "../cbusgetminorpriority.hpp"
#include "../cbuskernel.hpp"
#include "../messages/cbusmessage.hpp"

namespace {

std::array<CAN::SocketCANIOHandler::Filter, 1> filter{{
  // only standard non RTR frames
  {
	  .can_id = 0,
	  .can_mask = (CAN_EFF_FLAG | CAN_RTR_FLAG),
  }
}};

}

namespace CBUS {

SocketCANIOHandler::SocketCANIOHandler(Kernel& kernel, const std::string& interface)
  : IOHandler(kernel)
  , m_socketCAN{kernel.ioContext(), interface, m_kernel.logId,
      [this](const CAN::SocketCANIOHandler::Frame& frame)
      {
        if(frame.can_dlc >= 1) // only with minimal 1 data byte
        {
          CAN::Message canMessage;
          canMessage.id = frame.can_id & CAN_EFF_MASK;
          canMessage.rtr = (frame.can_id & CAN_RTR_FLAG) != 0;
          canMessage.extended = (frame.can_id & CAN_EFF_FLAG) != 0;
          std::memcpy(canMessage.data, frame.data, frame.can_dlc);
          assert(onReceive);
          onReceive(canMessage);
        }
      },
      std::bind(&Kernel::error, &m_kernel),
      filter}
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

std::error_code SocketCANIOHandler::send(const CAN::Message& canMessage)
{
  CAN::SocketCANIOHandler::Frame frame;
  frame.can_id = canMessage.id;
  if(canMessage.rtr)
  {
    frame.can_id |= CAN_RTR_FLAG;
  }
  if(canMessage.extended)
  {
    frame.can_id |= CAN_RTR_FLAG;
  }
  frame.can_dlc = canMessage.dlc;
  std::memcpy(frame.data, &canMessage.data, canMessage.dlc);
  if(!m_socketCAN.send(frame))
  {
    return std::make_error_code(std::errc::no_buffer_space);
  }
  return {};
}

}
