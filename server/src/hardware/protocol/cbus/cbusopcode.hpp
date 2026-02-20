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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_CBUSOPCODE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_CBUSOPCODE_HPP

#include <cstdint>

namespace CBUS {

//! \brief CBUS OpCode
//! \see https://github.com/cbus-traincontrol/cbus-traincontrol.github.io/blob/main/CBUS%20Developers%20Guide%206c.pdf
//!
enum class OpCode : uint8_t
{
  // 00-1F – 0 data bytes packets:
  ACK    = 0x00, //!< General Acknowledgement
  NAK    = 0x01, //!< General No Ack
  HLT    = 0x02, //!< Bus Halt
  BON    = 0x03, //!< Bus ON
  TOF    = 0x04, //!< Track OFF
  TON    = 0x05, //!< Track ON
  ESTOP  = 0x06, //!< Emergency Stop
  ARST   = 0x07, //!< System Reset
  RTOF   = 0x08, //!< Request Track OFF
  RTON   = 0x09, //!< Request Track ON
  RESTP  = 0x0A, //!< Request Emergency Stop ALL
  RSTAT  = 0x0C, //!< Request Command Station Status
  QNN    = 0x0D, //!< Query node number
  RQNP   = 0x10, //!< Request node parameters
  RQMN   = 0x11, //!< Request module name

  // 20–3F - 1 data byte packets:
  KLOC   = 0x21, //!< Release Engine
  QLOC   = 0x22, //!< Query engine
  DKEEP  = 0x23, //!< Session keep alive
  DBG1   = 0x30, //!< Debug with one data byte
  EXTC   = 0x3F, //!< Extended op-code with no additional bytes

  // 40–5F - 2 data byte packets:
  RLOC   = 0x40, //!< Request engine session
  QCON   = 0x41, //!< Query Consist
  SNN    = 0x42, //!< Set Node Number
  ALOC   = 0x43, //!< Allocate loco to activity
  STMOD  = 0x44, //!< Set CAB session mode
  PCON   = 0x45, //!< Consist Engine
  KCON   = 0x46, //!< Remove Engine from consist
  DSPD   = 0x47, //!< Set Engine Speed/Dir
  DFLG   = 0x48, //!< Set Engine Flags
  DFNON  = 0x49, //!< Set Engine function on
  DFNOF  = 0x4A, //!< Set Engine function off
  SSTAT  = 0x4C, //!< Service mode status
  NNRSM  = 0x4F, //!< Reset to manufacturers defaults
  RQNN   = 0x50, //!< Request node number
  NNREL  = 0x51, //!< Node number release
  NNACK  = 0x52, //!< Node number acknowledge
  NNLRN  = 0x53, //!< Set node into learn mode
  NNULN  = 0x54, //!< Release node from learn mode
  NNCLR  = 0x55, //!< Clear all events from a node
  NNEVN  = 0x56, //!< Read number of events available in a node
  NERD   = 0x57, //!< Read back all stored events in a node
  RQEVN  = 0x58, //!< Request to read number of stored events
  WRACK  = 0x59, //!< Write acknowledge
  RQDAT  = 0x5A, //!< Request node data event
  RQDDS  = 0x5B, //!< Request device data – short mode
  BOOTM  = 0x5C, //!< Put node into bootload mode
  ENUM   = 0x5D, //!< Force a self enumeration cycle for use with CAN
  NNRST  = 0x5E, //!< Restart node
  EXTC1  = 0x5F, //!< Extended op-code with 1 additional byte

  // 60-7F - 3 data byte packets:
  DFUN   = 0x60, //!< Set Engine functions
  GLOC   = 0x61, //!< Get engine session
  ERR    = 0x63, //!< Command Station Error report
  CMDERR = 0x6F, //!< Error messages from nodes during configuration
  EVNLF  = 0x70, //!< Event space left reply from node
  NVRD   = 0x71, //!< Request read of a node variable
  NENRD  = 0x72, //!< Request read of stored events by event index
  RQNPN  = 0x73, //!< Request read of a node parameter by index
  NUMEV  = 0x74, //!< Number of events stored in node
  CANID  = 0x75, //!< Set a CAN_ID in existing FLiM node
  EXTC2  = 0x7F, //!< Extended op-code with 2 additional bytes

  // 80-9F - 4 data byte packets:
  RDCC3  = 0x80, //!< Request 3-byte DCC Packet
  WCVO   = 0x82, //!< Write CV (byte) in OPS mode
  WCVB   = 0x83, //!< Write CV (bit) in OPS mode
  QCVS   = 0x84, //!< Read CV
  PCVS   = 0x85, //!< Report CV
  ACON   = 0x90, //!< Accessory ON
  ACOF   = 0x91, //!< Accessory OFF
  AREQ   = 0x92, //!< Accessory Request Event
  ARON   = 0x93, //!< Accessory Response Event
  AROF   = 0x94, //!< Accessory Response Event
  EVULN  = 0x95, //!< Unlearn an event in learn mode
  NVSET  = 0x96, //!< Set a node variable
  NVANS  = 0x97, //!< Response to a request for a node variable value
  ASON   = 0x98, //!< Accessory Short ON
  ASOF   = 0x99, //!< Accessory Short OFF
  ASRQ   = 0x9A, //!< Accessory Short Request Event
  PARAN  = 0x9B, //!< Response to request for individual node parameter
  REVAL  = 0x9C, //!< Request for read of an event variable
  ARSON  = 0x9D, //!< Accessory Short Response Event
  ARSOF  = 0x9E, //!< Accessory Short Response Event
  EXTC3  = 0x9F, //!< Extended op-code with 3 additional bytes

  // A0-BF - 5 data byte packets:
  RDCC4  = 0xA0, //!< Request 4-byte DCC Packet
  WCVS   = 0xA2, //!< Write CV in Service mode
  ACON1  = 0xB0, //!< Accessory ON
  ACOF1  = 0xB1, //!< Accessory OFF
  REQEV  = 0xB2, //!< Read event variable in learn mode
  ARON1  = 0xB3, //!< Accessory Response Event
  AROF1  = 0xB4, //!< Accessory Response Event
  NEVAL  = 0xB5, //!< Response to request for read of EV value
  PNN    = 0xB6, //!< Response to Query Node
  ASON1  = 0xB8, //!< Accessory Short ON
  ASOF1  = 0xB9, //!< Accessory Short OFF
  ARSON1 = 0xBD, //!< Accessory Short Response Event with one data byte
  ARSOF1 = 0xBE, //!< Accessory Short Response Event with one data byte
  EXTC4  = 0xBF, //!< Extended op-code with 4 data bytes

  // C0-DF - 6 data byte packets:
  RDCC5  = 0xC0, //!< Request 5-byte DCC Packet
  WCVOA  = 0xC1, //!< Write CV (byte) in OPS mode by address
  CABDAT = 0xC2, //!< Cab Data
  FCLK   = 0xCF, //!< Fast Clock

  // E0-FF - 7 data byte packets:
  RDCC6  = 0xE0, //!< Request 6-byte DCC Packet
  PLOC   = 0xE1, //!< Engine report
  NAME   = 0xE2, //!< Response to request for node name string
  STAT   = 0xE3, //!< Command Station status report
  PARAMS = 0xEF, //!< Response to request for node parameters
  ACON3  = 0xF0, //!< Accessory ON
  ACOF3  = 0xF1, //!< Accessory OFF
  ENRSP  = 0xF2, //!< Response to request to read node events
  ARON3  = 0xF3, //!< Accessory Response Event
  AROF3  = 0xF4, //!< Accessory Response Event
  EVLRNI = 0xF5, //!< Teach an event in learn mode using event indexing
  ACDAT  = 0xF6, //!< Accessory node data event
  ARDAT  = 0xF7, //!< Accessory node data Response
  ASON3  = 0xF8, //!< Accessory Short ON
  ASOF3  = 0xF9, //!< Accessory Short OFF
  DDES   = 0xFA, //!< Device data event (short mode)
  DDRS   = 0xFB, //!< Device data response (short mode)
  ARSON3 = 0xFD, //!< Accessory Short Response Event
  ARSOF3 = 0xFE, //!< Accessory Short Response Event
  EXTC6  = 0xFF, //!< Extended op-code with 6 data bytes
};

constexpr uint8_t dataSize(OpCode opc)
{
  return static_cast<uint8_t>(opc) >> 5; // highest 3 bits determine data length
}

}

#endif
