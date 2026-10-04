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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_SIMULATOR_CBUSSIMULATOR_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_SIMULATOR_CBUSSIMULATOR_HPP

#include <memory>
#include <map>
#include <vector>
#include "module/cbuscanmodule.hpp"
#include "../messages/cbusmessage.hpp"
#include "../../can/canmessage.hpp"
#include "../../../../enum/simulateinputaction.hpp"

namespace CBUS
{

class Simulator
{
  friend class Module::CANModule;

public:
  std::function<void(const CAN::Message&)> onSend;

  Simulator();

  bool addModule(std::unique_ptr<Module::CANModule> module);

  void receive(const CAN::Message& canMessage);

  void shortEvent(uint16_t eventNumber, SimulateInputAction action);
  void longEvent(uint16_t nodeNumber, uint16_t eventNumber, SimulateInputAction action);

private:
  std::vector<std::unique_ptr<Module::CANModule>> m_modules;
  std::map<uint16_t, bool> m_shortEvents;
  std::map<std::pair<uint16_t, uint16_t>, bool> m_longEvents;

  void send(const CAN::Message& message);
};

}

#endif
