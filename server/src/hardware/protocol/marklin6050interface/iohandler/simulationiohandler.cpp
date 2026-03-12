/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Kamil Kasprzak
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
#include "../protocol.hpp"

namespace Marklin6050 {

SimulationIOHandler::SimulationIOHandler(Kernel& kernel,
                                         boost::asio::io_context::strand& strand)
  : IOHandler{kernel}
  , m_strand{strand}
{
}

void SimulationIOHandler::start()
{
  started(); // delegates to IOHandler::started() → m_kernel.started()
}

void SimulationIOHandler::stop()
{
  // nothing to close
}

void SimulationIOHandler::send(std::initializer_list<uint8_t> bytes)
{
  if(bytes.size() != 1)
  {
    return; // only single-byte commands trigger a simulated response
  }

  const uint8_t b = *bytes.begin();

  if(b >= S88Base)
  {
    // S88 poll: respond with (moduleCount * 2) zero bytes — all contacts clear
    const uint8_t moduleCount = b - S88Base;
    m_strand.post(
      [this, moduleCount]()
      {
        for(uint8_t i = 0; i < moduleCount * 2u; ++i)
          m_kernel.receive(0x00);
      });
  }
  // All other single-byte commands (GlobalGo, GlobalStop, Extension poll)
  // are silently consumed — no response needed.
}

} // namespace Marklin6050
