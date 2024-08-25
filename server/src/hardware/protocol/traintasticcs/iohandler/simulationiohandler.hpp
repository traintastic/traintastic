/**
 * server/src/hardware/protocol/traintasticcs/iohandler/simulationiohandler.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_TRAINTASTICCS_IOHANDLER_SIMULATIONIOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_TRAINTASTICCS_IOHANDLER_SIMULATIONIOHANDLER_HPP

#include "iohandler.hpp"
#include <vector>
#include <cstddef>
#include <cstdint>

enum class SimulateInputAction;

namespace TraintasticCS {

enum class InputChannel : uint8_t;
enum class InputState : uint8_t;

class SimulationIOHandler final : public IOHandler
{
  private:
    bool m_initXpressNet = false;
    struct S88
    {
      bool enabled = false;
      std::vector<InputState> states;

      void init(uint8_t moduleCount);
      void reset();
    } m_s88;

    void reply(const Message& message);

  public:
    SimulationIOHandler(Kernel& kernel);

    void start() final;
    void stop() final {}

    bool send(const Message& message) final;

    void inputSimulateChange(InputChannel channel, uint16_t address, SimulateInputAction action);
};

template<>
constexpr bool isSimulation<SimulationIOHandler>()
{
  return true;
}

}

#endif

