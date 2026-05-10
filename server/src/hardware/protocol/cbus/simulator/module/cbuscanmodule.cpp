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

#include "cbuscanmodule.hpp"
#include "../cbussimulator.hpp"
#include "../../cbuscanmessageutils.hpp"

namespace CBUS::Module {

CANModule::CANModule(Simulator& simulator, uint16_t nodeNumber_, uint8_t canId_)
  : nodeNumber{nodeNumber_}
  , canId{canId_}
  , m_simulator{simulator}
{
}

void CANModule::send(const CAN::Message& canMessage)
{
  m_simulator.send(canMessage);
}

void CANModule::send(const Message& message)
{
  send(toCANMessage(message, canId));
}

}
