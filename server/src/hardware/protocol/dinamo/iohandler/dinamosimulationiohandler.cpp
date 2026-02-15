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

#include "dinamosimulationiohandler.hpp"
#include <vector>
#include "../dinamoerror.hpp"
#include "../dinamokernel.hpp"
#include "../dinamomessages.hpp"
#include "../simulator/dinamosimulator.hpp"

namespace Dinamo {

SimulationIOHandler::SimulationIOHandler(Kernel& kernel, Simulator& simulator)
  : IOHandler(kernel)
  , m_simulator{simulator}
{
}

SimulationIOHandler::~SimulationIOHandler() = default;

std::error_code SimulationIOHandler::send(std::span<const uint8_t> message, bool hold, bool fault)
{
  if(message.size() > maxMessageSize) [[unlikely]]
  {
    assert(false);
    return Error::messageTooLarge();
  }

  m_timer.cancel();

  // cache last sent values, in case we need to send a Null datagram:
  m_txHold = hold;
  m_txFault = fault;

  const auto& [response, rxHold, rxFault] = m_simulator.process(message, hold, fault);

  if(!response.empty() || hold != m_rxHold || fault != m_rxFault) // only if not NULL or hold/fault is changed
  {
    reply(response, hold, fault);
  }
  else
  {
    startIdleTimeoutTimer();
  }

  m_rxHold = rxHold;
  m_rxFault = rxFault;

  return {};
}

void SimulationIOHandler::reply(std::span<const uint8_t> message, bool hold, bool fault)
{
  // post the reply, so it has some delay
  //! \todo better delay simulation? at least DINAMO message transfer time?
  m_kernel.ioContext().post(
    [this, data=std::vector<uint8_t>(message.begin(), message.end()), hold, fault]()
    {
      startIdleTimeoutTimer();
      m_kernel.receive(data, hold, fault);
    });
}

}
