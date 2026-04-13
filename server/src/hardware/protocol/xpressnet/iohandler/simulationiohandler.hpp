/**
 * server/src/hardware/protocol/xpressnet/iohandler/simulationiohandler.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2024 Reinder Feenstra
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
#include "../messages.hpp"
#include <unordered_map>
#include <array>
#include <cstddef>

namespace XpressNet {

class SimulationIOHandler final : public IOHandler
{
  private:
    struct Locomotive
    {
      LocomotiveInfo info;
      FunctionInfoF13F28 func13;
      FunctionInfoF29F68 func29;

      Locomotive()
      {
        info.setDirection(Direction::Forward);
        info.setEmergencyStop();
        info.setBusy(false);
        info.setSpeedSteps(126);
        info.updateChecksum();
        func13.updateChecksum();
        func29.updateChecksum();
      }
    };

    std::unordered_map<uint16_t, Locomotive> m_locomotives;

    void reply(const Message& message);
    void reply(const Message& message, size_t count);

    void speedAndDirectionInstruction(const SpeedAndDirectionInstruction& message);

  public:
    SimulationIOHandler(Kernel& kernel);

    void start() final;
    void stop() final {}

    bool send(const Message& message) final;
};

template<>
constexpr bool isSimulation<SimulationIOHandler>()
{
  return true;
}

}

#endif

