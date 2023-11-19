/**
 * server/src/hardware/protocol/selectrix/iohandler/simulationiohandler.cpp
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

#include "simulationiohandler.hpp"
#include <cstddef>
#include <cassert>
#include <thread>

namespace Selectrix {

constexpr auto ioDelay = std::chrono::microseconds(500);

SimulationIOHandler::SimulationIOHandler(Kernel& kernel)
  : IOHandler(kernel)
{
  for(auto& values : m_busValues)
  {
    std::fill(values.begin(), values.end(), 0);
  }
}

bool SimulationIOHandler::read(uint8_t address, uint8_t& value)
{
  assert(address <= Address::max);
  std::this_thread::sleep_for(ioDelay);
  if(address == Address::selectSXBus)
  {
    value = static_cast<uint8_t>(m_bus);
  }
  else
  {
    value = busValues()[address];
  }
  return true;
}

bool SimulationIOHandler::write(uint8_t address, uint8_t value)
{
  assert(address <= Address::max);
  std::this_thread::sleep_for(ioDelay);
  if(address == Address::selectSXBus)
  {
    m_bus = static_cast<Bus>(value);
  }
  else
  {
    busValues()[address] = value;
  }
  return true;
}

}
