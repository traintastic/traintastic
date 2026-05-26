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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_MESSAGES_CBUSGENERALMESSAGES_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_MESSAGES_CBUSGENERALMESSAGES_HPP

#include "cbusmessage.hpp"

namespace CBUS {

template<OpCode opc>
struct GeneralMessage : Message
{
  GeneralMessage()
    : Message(opc)
  {
    static_assert(static_cast<uint8_t>(opc) <= 0x1F); // zero data byte opcodes only.
  }
};

using Ack = GeneralMessage<OpCode::ACK>;
using NoAck = GeneralMessage<OpCode::NAK>;
using BusHalt = GeneralMessage<OpCode::HLT>;
using BusOn = GeneralMessage<OpCode::BON>;
using TrackOff = GeneralMessage<OpCode::TOF>;
using TrackOn = GeneralMessage<OpCode::TON>;
using EmergencyStop = GeneralMessage<OpCode::ESTOP>;
using SystemReset = GeneralMessage<OpCode::ARST>;
using RequestTrackOff = GeneralMessage<OpCode::RTOF>;
using RequestTrackOn = GeneralMessage<OpCode::RTON>;
using RequestEmergencyStop = GeneralMessage<OpCode::RESTP>;
using RequestCommandStationStatus = GeneralMessage<OpCode::RSTAT>;
using QueryNodeNumber = GeneralMessage<OpCode::QNN>;
using RequestNodePrameters = GeneralMessage<OpCode::RQNP>;
using RequestModuleName = GeneralMessage<OpCode::RQMN>;

}

#endif
