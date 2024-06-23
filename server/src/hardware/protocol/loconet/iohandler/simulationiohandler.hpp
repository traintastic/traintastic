/**
 * server/src/hardware/protocol/loconet/iohandler/simulationiohandler.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_IOHANDLER_SIMULATIONIOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_IOHANDLER_SIMULATIONIOHANDLER_HPP

#include "iohandler.hpp"
#include <array>
#include <vector>
#include <unordered_map>
#include "../messages.hpp"

namespace LocoNet {

class SimulationIOHandler final : public IOHandler
{
  private:
    struct LNCVModule
    {
      static constexpr uint16_t broadcastAddress = 65535;
      static constexpr uint16_t lncvAddress = 0;

      const uint16_t id;
      bool programmingModeActive;
      std::unordered_map<uint16_t, uint16_t> lncvs;

      uint16_t address() const
      {
        assert(lncvs.find(LNCVModule::lncvAddress) != lncvs.end());
        return lncvs.find(LNCVModule::lncvAddress)->second;
      }
    };

    std::array<SlotReadData, SLOT_LOCO_MAX - SLOT_LOCO_MIN + 1> m_locoSlots;
    std::vector<LNCVModule> m_lncvModules;

    void reply(const Message& message);

  public:
    SimulationIOHandler(Kernel& kernel);

    void start() final
    {
      started();
    }

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

