/**
 * server/src/hardware/protocol/xpressnet/iohandler/simulationiohandler.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_XPRESSNET_IOHANDLER_SIMULATIONIOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_XPRESSNET_IOHANDLER_SIMULATIONIOHANDLER_HPP

#include "iohandler.hpp"
#include <array>
#include <cstddef>
#include "../../../../simulator/simulatoriohandler.hpp"

enum class SimulateInputAction;

namespace XpressNet {

class SimulationIOHandler final : public IOHandler
{
  private:
    std::unique_ptr<SimulatorIOHandler> m_simulator;
    std::unordered_map<uint16_t, bool> m_inputs;

    void reply(const Message& message);
    void reply(const Message& message, size_t count);

  public:
    SimulationIOHandler(Kernel& kernel);

    void setSimulator(std::string hostname, uint16_t port);

    void start() final;
    void stop() final {}

    bool send(const Message& message) final;

    void simulateInputChange(uint16_t address, SimulateInputAction action);
};

template<>
constexpr bool isSimulation<SimulationIOHandler>()
{
  return true;
}

}

#endif

