/**
 * server/src/hardware/protocol/selectrix/iohandler/simulationiohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023,2025 Reinder Feenstra
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
#include <cstddef>
#include <cassert>
#include "../kernel.hpp"
#include "../utils.hpp"
#include "../../../../enum/simulateinputaction.hpp"

namespace Selectrix {

SimulationIOHandler::SimulationIOHandler(Kernel& kernel)
  : IOHandler(kernel)
{
  for(auto& values : m_busValues)
  {
    std::fill(values.begin(), values.end(), 0);
  }
}

bool SimulationIOHandler::read(Bus bus, uint8_t address)
{
  assert(address <= Address::max);
  m_kernel.ioContext().post(
    [this, bus, address]()
    {
      m_kernel.busChanged(bus, address, busValues(bus)[address]);
    });
  return true;
}

bool SimulationIOHandler::write(Bus bus, uint8_t address, uint8_t value)
{
  assert(address <= Address::max);
  busValues(bus)[address] = value;
  return true;
}

void SimulationIOHandler::simulateInputChange(Bus bus, uint16_t inputAddress, SimulateInputAction action)
{
  assert(inputAddress > 0);
  const uint8_t address = toBusAddress(inputAddress);
  const uint8_t valueMask = 1 << toPort(inputAddress);

  const bool setBit =
    (action == SimulateInputAction::SetTrue) ||
    (action == SimulateInputAction::Toggle && !(busValues(bus)[address] & valueMask));

  if(setBit)
  {
    busValues(bus)[address] |= valueMask;
  }
  else // clear bit
  {
    busValues(bus)[address] &= ~valueMask;
  }

  m_kernel.ioContext().post(
    [this, bus, address]()
    {
      m_kernel.busChanged(bus, address, busValues(bus)[address]);
    });
}

}
