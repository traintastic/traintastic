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

#include "rclinksimulationiohandler.hpp"
#include <vector>
#include <boost/asio/post.hpp>
#include "../rclinkkernel.hpp"
#include "../simulator/rclinksimulator.hpp"

namespace RCLink {

SimulationIOHandler::SimulationIOHandler(Kernel& kernel, Simulator& simulator)
  : IOHandler(kernel)
  , m_simulator{simulator}
{
  m_simulator.onSend =
    [this](std::span<const uint8_t> message)
    {
      // post the message, so it has some delay
      boost::asio::post(m_kernel.ioContext(),
        [this, data=std::vector<uint8_t>(message.begin(), message.end())]()
        {
          m_kernel.receive(data);
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

std::error_code SimulationIOHandler::send(std::span<const uint8_t> message)
{
  m_simulator.receive(message);
  return {};
}

}
