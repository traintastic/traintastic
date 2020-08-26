/**
 * server/src/hardware/protocol/loconet/opcode.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

/**
 * Portions Copyright (C) Digitrax Inc.
 *
 * LocoNet is a registered trademark of DigiTrax, Inc.
 */

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_OPCODE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_OPCODE_HPP

#include <cstdint>
#include <string_view>

namespace LocoNet {

enum OpCode : uint8_t
{
  // 2 byte message opcodes:
  OPC_BUSY = 0x81,
  OPC_GPOFF = 0x82,
  OPC_GPON = 0x83,
  OPC_IDLE = 0x85,

  // 4 byte message opcodes:
  OPC_LOCO_SPD = 0xA0,
  OPC_LOCO_DIRF = 0xA1,
  OPC_LOCO_SND = 0xA2,
  OPC_LOCO_F9F12 = 0xA3, // based on reverse engineering, see loconet.md
  OPC_SW_REQ = 0xB0,
  OPC_SW_REP = 0xB1,
  OPC_INPUT_REP = 0xB2,
  OPC_LONG_ACK = 0xB4,
  OPC_SLOT_STAT1 = 0xB5,
  OPC_CONSIST_FUNC = 0xB6,
  OPC_UNLINK_SLOTS = 0xB8,
  OPC_LINK_SLOTS = 0xB9,
  OPC_MOVE_SLOTS = 0xBA,
  OPC_RQ_SL_DATA = 0xBB,
  OPC_SW_STATE = 0xBC,
  OPC_SW_ACK = 0xBD,
  OPC_LOCO_ADR = 0xBF,

  // 6 byte message opcodes:
  OPC_MULTI_SENSE = 0xD0, // based on reverse engineering, see loconet.md
  OPC_D4 = 0xD4,// based on reverse engineering, probably used for multiple sub commands, see loconet.md

  // variable byte message opcodes:
  OPC_MULTI_SENSE_LONG = 0XE0, // based on reverse engineering, see loconet.md
  OPC_PEER_XFER = 0xE5,
  OPC_SL_RD_DATA = 0xE7,
  OPC_IMM_PACKET = 0xED,
  OPC_WR_SL_DATA = 0xEF,
};

constexpr std::string_view toString(OpCode value)
{
  switch(value)
  {
    case LocoNet::OPC_BUSY: return "OPC_BUSY";
    case LocoNet::OPC_GPOFF: return "OPC_GPOFFqqq";
    case LocoNet::OPC_GPON: return "OPC_GPON";
    case LocoNet::OPC_IDLE: return "OPC_IDLE";
    case LocoNet::OPC_LOCO_SPD: return "OPC_LOCO_SPD";
    case LocoNet::OPC_LOCO_DIRF: return "OPC_LOCO_DIRF";
    case LocoNet::OPC_LOCO_SND: return "OPC_LOCO_SND";
    case LocoNet::OPC_SW_REQ: return "OPC_SW_REQ";
    case LocoNet::OPC_SW_REP: return "OPC_SW_REP";
    case LocoNet::OPC_INPUT_REP: return "OPC_INPUT_REP";
    case LocoNet::OPC_LONG_ACK: return "OPC_LONG_ACK";
    case LocoNet::OPC_SLOT_STAT1: return "OPC_SLOT_STAT1";
    case LocoNet::OPC_CONSIST_FUNC: return "OPC_CONSIST_FUNC";
    case LocoNet::OPC_UNLINK_SLOTS: return "OPC_UNLINK_SLOTS";
    case LocoNet::OPC_LINK_SLOTS: return "OPC_LINK_SLOTS";
    case LocoNet::OPC_MOVE_SLOTS: return "OPC_MOVE_SLOTS";
    case LocoNet::OPC_RQ_SL_DATA: return "OPC_RQ_SL_DATA";
    case LocoNet::OPC_SW_STATE: return "OPC_SW_STATE";
    case LocoNet::OPC_SW_ACK: return "OPC_SW_ACK";
    case LocoNet::OPC_LOCO_ADR: return "OPC_LOCO_ADR";
    case LocoNet::OPC_MULTI_SENSE: return "OPC_MULTI_SENSE";
    case LocoNet::OPC_PEER_XFER: return "OPC_PEER_XFER";
    case LocoNet::OPC_SL_RD_DATA: return "OPC_SL_RD_DATA";
    case LocoNet::OPC_IMM_PACKET: return "OPC_IMM_PACKET";
    case LocoNet::OPC_WR_SL_DATA: return "OPC_WR_SL_DATA";
  }

  return {};
}

}

#endif
