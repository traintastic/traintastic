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

#include "cbussimulationiohandler.hpp"
#include <boost/asio/post.hpp>
#include "../cbusconst.hpp"
#include "../cbuskernel.hpp"
#include "../simulator/cbussimulator.hpp"

namespace CBUS {

SimulationIOHandler::SimulationIOHandler(Kernel& kernel, Simulator& simulator)
  : IOHandler(kernel)
  , m_simulator{simulator}
{
  m_simulator.onSend =
    [this](const CAN::Message& canMessage)
    {
      // post the message, so it has some delay
      boost::asio::post(m_kernel.ioContext(),
        [this, canMessage]()
        {
          assert(onReceive);
          onReceive(canMessage);
        });
    };
}

SimulationIOHandler::~SimulationIOHandler() = default;

void SimulationIOHandler::start()
{
  m_kernel.started();
}

void SimulationIOHandler::stop()
{
}

std::error_code SimulationIOHandler::send(const CAN::Message& canMessage)
{
  m_simulator.receive(canMessage);
  return {};
}

}
