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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DINAMO_IOHANDLER_DINAMOSIMULATIONIOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DINAMO_IOHANDLER_DINAMOSIMULATIONIOHANDLER_HPP

#include "dinamoiohandler.hpp"
#include <memory>

namespace Dinamo {

class Simulator;

class SimulationIOHandler final : public IOHandler
{
public:
  SimulationIOHandler(Kernel& kernel, Simulator& simulator);
  ~SimulationIOHandler() final;

  void stop() final {}

  [[nodiscard]] std::error_code send(std::span<const uint8_t> message, bool hold, bool fault) final;

private:
  Simulator& m_simulator;
  bool m_hold = false;
  bool m_fault = true;

  void reply(std::span<const uint8_t> message, bool hold, bool fault);
};

template<>
constexpr bool isSimulation<SimulationIOHandler>()
{
  return true;
}

}

#endif

