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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_SIMULATOR_MODULE_CBUSCANCMD_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_SIMULATOR_MODULE_CBUSCANCMD_HPP

#include "cbuscanmodule.hpp"
#include "../../messages/cbusenginemessages.hpp"
#include <map>

namespace CBUS::Module {

class CANCMD : public CANModule
{
public:
  CANCMD(Simulator& simulator, uint16_t nodeNumber_ = NodeNumber::CANCMD, uint8_t canId_ = 0x72);

  void receive(const CAN::Message& canMessage) override;

private:
  bool m_busOn = true;
  bool m_trackOn = false;
  bool m_eStop = true;
  std::map<uint8_t, EngineReport> m_sessions;

  EngineReport* newEngineSession(const EngineReport& init);
  void handleGetEngineSession(uint16_t address, bool isLongAddress, GetEngineSession::Mode mode);
};

}

#endif
