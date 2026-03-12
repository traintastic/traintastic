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

namespace Marklin6023 {

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

void SimulationIOHandler::sendString(std::string str)
{
  // Respond to S88 contact queries with "0" (clear) so the polling
  // cycle runs normally without real hardware. Format: "C <n>\r"
  if(str.size() >= 3 && str[0] == 'C' && str[1] == ' ')
  {
    m_strand.post(
      [this]()
      {
        m_kernel.receiveLine("0");
      });
  }
  // All other commands (G, S, L ..., M ...) are silently consumed.
}

} // namespace Marklin6023
