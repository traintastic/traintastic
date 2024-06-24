/**
 * server/src/hardware/protocol/ecos/iohandler/simulationiohandler.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022,2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_IOHANDLER_SIMULATIONIOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_IOHANDLER_SIMULATIONIOHANDLER_HPP

#include "iohandler.hpp"
#include <array>
#include <cstddef>
#include "../simulation.hpp"

namespace ECoS {

class SimulationIOHandler final : public IOHandler
{
  private:
    const Simulation m_simulation;

    bool reply(std::string_view message);
    bool replyOk(std::string_view request);
    bool replyErrorUnknownOption(std::string_view request, std::string_view option);

    static std::string replyHeader(std::string_view request);

  public:
    SimulationIOHandler(Kernel& kernel, const Simulation& simulation);

    void start() final;
    void stop() final {}

    bool send(std::string_view message) final;
};

template<>
constexpr bool isSimulation<SimulationIOHandler>()
{
  return true;
}

}

#endif

