/**
 * server/src/hardware/protocol/traintasticcs/iohandler/simulationiohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#include "simulationiohandler.hpp"
#include "../kernel.hpp"
#include "../messages.hpp"
#include <version.hpp>

namespace TraintasticCS {

static std::shared_ptr<std::byte[]> copy(const Message& message)
{
  auto* bytes = new std::byte[message.size()];
  std::memcpy(bytes, &message, message.size());
  return std::shared_ptr<std::byte[]>{bytes};
}

SimulationIOHandler::SimulationIOHandler(Kernel& kernel)
  : IOHandler(kernel)
{
}

void SimulationIOHandler::start()
{
  m_kernel.started();
}

bool SimulationIOHandler::send(const Message& message)
{
  switch(message.command)
  {
    case Command::Ping:
      reply(Pong());
      break;

    case Command::GetInfo:
      reply(Info(Board::TraintasticCS, 0, 1, 0));
      break;

    case Command::Pong:
    case Command::Info:
      assert(false); // only send by device
      break;
  }

  return true;
}

void SimulationIOHandler::reply(const Message& message)
{
  // post the reply, so it has some delay
  //! \todo better delay simulation? at least message transfer time?
  m_kernel.ioContext().post(
    [this, data=copy(message)]()
    {
      m_kernel.receive(*reinterpret_cast<const Message*>(data.get()));
    });
}

}
