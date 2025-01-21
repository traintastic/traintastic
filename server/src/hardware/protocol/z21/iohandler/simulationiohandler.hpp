/**
 * server/src/hardware/protocol/z21/iohandler/simulationiohandler.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_Z21_IOHANDLER_SIMULATIONIOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_Z21_IOHANDLER_SIMULATIONIOHANDLER_HPP

#include "iohandler.hpp"
#include <array>
#include <unordered_map>
#include "../messages.hpp"

namespace Z21 {

class SimulationIOHandler final : public IOHandler
{
  private:
    static constexpr uint8_t firmwareVersionMajor = 1;
    static constexpr uint8_t firmwareVersionMinor = 30;
    static constexpr HardwareType hardwareType = HWT_Z21_NEW;
    static constexpr uint32_t serialNumber = 123456789;
    static constexpr uint8_t xBusVersion = 30;

    bool m_emergencyStop = false;
    bool m_trackPowerOn = false;
    BroadcastFlags m_broadcastFlags = BroadcastFlags::None;

    std::unordered_map<uint16_t, LanXLocoInfo> m_decoderCache;

    void reply(const Message& message);
    void replyLanSystemStateDataChanged();

  public:
    SimulationIOHandler(Kernel& kernel);

    void start() final {}
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

