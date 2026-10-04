--[[

CBUS/VLCB protocol dissector for WireShark

Copyright (c) 2026 Reinder Feenstra

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

]]

local default_settings =
{
  port = 5550
}

local cbus = Proto("cbus", "CBUS/VLCB protocol")

-- 00-1F – 0 data bytes packets:
local OPC_ACK    = 0x00 -- General Acknowledgement
local OPC_NAK    = 0x01 -- General No Ack
local OPC_HLT    = 0x02 -- Bus Halt
local OPC_BON    = 0x03 -- Bus ON
local OPC_TOF    = 0x04 -- Track OFF
local OPC_TON    = 0x05 -- Track ON
local OPC_ESTOP  = 0x06 -- Emergency Stop
local OPC_ARST   = 0x07 -- System Reset
local OPC_RTOF   = 0x08 -- Request Track OFF
local OPC_RTON   = 0x09 -- Request Track ON
local OPC_RESTP  = 0x0A -- Request Emergency Stop ALL
local OPC_RSTAT  = 0x0C -- Request Command Station Status
local OPC_QNN    = 0x0D -- Query node number
local OPC_RQNP   = 0x10 -- Request node parameters
local OPC_RQMN   = 0x11 -- Request module name

-- 20–3F - 1 data byte packets:
local OPC_KLOC   = 0x21 -- Release Engine
local OPC_QLOC   = 0x22 -- Query engine
local OPC_DKEEP  = 0x23 -- Session keep alive
local OPC_DBG1   = 0x30 -- Debug with one data byte
local OPC_EXTC   = 0x3F -- Extended op-code with no additional bytes

-- 40–5F - 2 data byte packets:
local OPC_RLOC   = 0x40 -- Request engine session
local OPC_QCON   = 0x41 -- Query Consist
local OPC_SNN    = 0x42 -- Set Node Number
local OPC_ALOC   = 0x43 -- Allocate loco to activity
local OPC_STMOD  = 0x44 -- Set CAB session mode
local OPC_PCON   = 0x45 -- Consist Engine
local OPC_KCON   = 0x46 -- Remove Engine from consist
local OPC_DSPD   = 0x47 -- Set Engine Speed/Dir
local OPC_DFLG   = 0x48 -- Set Engine Flags
local OPC_DFNON  = 0x49 -- Set Engine function on
local OPC_DFNOF  = 0x4A -- Set Engine function off
local OPC_SSTAT  = 0x4C -- Service mode status
local OPC_NNRSM  = 0x4F -- Reset to manufacturers defaults
local OPC_RQNN   = 0x50 -- Request node number
local OPC_NNREL  = 0x51 -- Node number release
local OPC_NNACK  = 0x52 -- Node number acknowledge
local OPC_NNLRN  = 0x53 -- Set node into learn mode
local OPC_NNULN  = 0x54 -- Release node from learn mode
local OPC_NNCLR  = 0x55 -- Clear all events from a node
local OPC_NNEVN  = 0x56 -- Read number of events available in a node
local OPC_NERD   = 0x57 -- Read back all stored events in a node
local OPC_RQEVN  = 0x58 -- Request to read number of stored events
local OPC_WRACK  = 0x59 -- Write acknowledge
local OPC_RQDAT  = 0x5A -- Request node data event
local OPC_RQDDS  = 0x5B -- Request device data – short mode
local OPC_BOOTM  = 0x5C -- Put node into bootload mode
local OPC_ENUM   = 0x5D -- Force a self enumeration cycle for use with CAN
local OPC_NNRST  = 0x5E -- Restart node
local OPC_EXTC1  = 0x5F -- Extended op-code with 1 additional byte

-- 60-7F - 3 data byte packets:
local OPC_DFUN   = 0x60 -- Set Engine functions
local OPC_GLOC   = 0x61 -- Get engine session
local OPC_ERR    = 0x63 -- Command Station Error report
local OPC_CMDERR = 0x6F -- Error messages from nodes during configuration
local OPC_EVNLF  = 0x70 -- Event space left reply from node
local OPC_NVRD   = 0x71 -- Request read of a node variable
local OPC_NENRD  = 0x72 -- Request read of stored events by event index
local OPC_RQNPN  = 0x73 -- Request read of a node parameter by index
local OPC_NUMEV  = 0x74 -- Number of events stored in node
local OPC_CANID  = 0x75 -- Set a CAN_ID in existing FLiM node
local OPC_EXTC2  = 0x7F -- Extended op-code with 2 additional bytes

-- 80-9F - 4 data byte packets:
local OPC_RDCC3  = 0x80 -- Request 3-byte DCC Packet
local OPC_WCVO   = 0x82 -- Write CV (byte) in OPS mode
local OPC_WCVB   = 0x83 -- Write CV (bit) in OPS mode
local OPC_QCVS   = 0x84 -- Read CV
local OPC_PCVS   = 0x85 -- Report CV
local OPC_ACON   = 0x90 -- Accessory ON
local OPC_ACOF   = 0x91 -- Accessory OFF
local OPC_AREQ   = 0x92 -- Accessory Request Event
local OPC_ARON   = 0x93 -- Accessory Response Event
local OPC_AROF   = 0x94 -- Accessory Response Event
local OPC_EVULN  = 0x95 -- Unlearn an event in learn mode
local OPC_NVSET  = 0x96 -- Set a node variable
local OPC_NVANS  = 0x97 -- Response to a request for a node variable value
local OPC_ASON   = 0x98 -- Accessory Short ON
local OPC_ASOF   = 0x99 -- Accessory Short OFF
local OPC_ASRQ   = 0x9A -- Accessory Short Request Event
local OPC_PARAN  = 0x9B -- Response to request for individual node parameter
local OPC_REVAL  = 0x9C -- Request for read of an event variable
local OPC_ARSON  = 0x9D -- Accessory Short Response Event
local OPC_ARSOF  = 0x9E -- Accessory Short Response Event
local OPC_EXTC3  = 0x9F -- Extended op-code with 3 additional bytes

-- A0-BF - 5 data byte packets:
local OPC_RDCC4  = 0xA0 -- Request 4-byte DCC Packet
local OPC_WCVS   = 0xA2 -- Write CV in Service mode
local OPC_ACON1  = 0xB0 -- Accessory ON
local OPC_ACOF1  = 0xB1 -- Accessory OFF
local OPC_REQEV  = 0xB2 -- Read event variable in learn mode
local OPC_ARON1  = 0xB3 -- Accessory Response Event
local OPC_AROF1  = 0xB4 -- Accessory Response Event
local OPC_NEVAL  = 0xB5 -- Response to request for read of EV value
local OPC_PNN    = 0xB6 -- Response to Query Node
local OPC_ASON1  = 0xB8 -- Accessory Short ON
local OPC_ASOF1  = 0xB9 -- Accessory Short OFF
local OPC_ARSON1 = 0xBD -- Accessory Short Response Event with one data byte
local OPC_ARSOF1 = 0xBE -- Accessory Short Response Event with one data byte
local OPC_EXTC4  = 0xBF -- Extended op-code with 4 data bytes

-- C0-DF - 6 data byte packets:
local OPC_RDCC5  = 0xC0 -- Request 5-byte DCC Packet
local OPC_WCVOA  = 0xC1 -- Write CV (byte) in OPS mode by address
local OPC_CABDAT = 0xC2 -- Cab Data
local OPC_FCLK   = 0xCF -- Fast Clock
local OPC_ACON2  = 0xD0 -- Accessory ON
local OPC_ACOF2  = 0xD1 -- Accessory OFF
local OPC_EVLRN  = 0xD2 -- Teach an event in learn mode
local OPC_EVANS  = 0xD3 -- Response to a request for an EV value in a node in learn mode
local OPC_ARON2  = 0xD4 -- Accessory Response Event
local OPC_AROF2  = 0xD5 -- Accessory Response Event
local OPC_ASON2  = 0xD8 -- Accessory Short ON
local OPC_ASOF2  = 0xD9 -- Accessory Short OFF
local OPC_ARSON2 = 0xDD -- Accessory Short Response Event with two data bytes
local OPC_ARSOF2 = 0xDE -- Accessory Short Response Event with two data bytes
local OPC_EXTC5  = 0xDF -- Extended op-code with 5 data bytes

-- E0-FF - 7 data byte packets:
local OPC_RDCC6  = 0xE0 -- Request 6-byte DCC Packet
local OPC_PLOC   = 0xE1 -- Engine report
local OPC_NAME   = 0xE2 -- Response to request for node name string
local OPC_STAT   = 0xE3 -- Command Station status report
local OPC_PARAMS = 0xEF -- Response to request for node parameters
local OPC_ACON3  = 0xF0 -- Accessory ON
local OPC_ACOF3  = 0xF1 -- Accessory OFF
local OPC_ENRSP  = 0xF2 -- Response to request to read node events
local OPC_ARON3  = 0xF3 -- Accessory Response Event
local OPC_AROF3  = 0xF4 -- Accessory Response Event
local OPC_EVLRNI = 0xF5 -- Teach an event in learn mode using event indexing
local OPC_ACDAT  = 0xF6 -- Accessory node data event
local OPC_ARDAT  = 0xF7 -- Accessory node data Response
local OPC_ASON3  = 0xF8 -- Accessory Short ON
local OPC_ASOF3  = 0xF9 -- Accessory Short OFF
local OPC_DDES   = 0xFA -- Device data event (short mode)
local OPC_DDRS   = 0xFB -- Device data response (short mode)
local OPC_ARSON3 = 0xFD -- Accessory Short Response Event
local OPC_ARSOF3 = 0xFE -- Accessory Short Response Event
local OPC_EXTC6  = 0xFF -- Extended op-code with 6 data bytes

local major_priorities = {
  [0] = "Highest",
  [1] = "Next",
  [2] = "Lowest",
}

local minor_priorities = {
  [0] = "High",
  [1] = "AboveNormal",
  [2] = "Normal",
  [3] = "Low",
}

local opcodes = {
  -- 00-1F – 0 data bytes packets:
  [OPC_ACK]    = "ACK",
  [OPC_NAK]    = "NAK",
  [OPC_HLT]    = "HLT",
  [OPC_BON]    = "BON",
  [OPC_TOF]    = "TOF",
  [OPC_TON]    = "TON",
  [OPC_ESTOP]  = "ESTOP",
  [OPC_ARST]   = "ARST",
  [OPC_RTOF]   = "RTOF",
  [OPC_RTON]   = "RTON",
  [OPC_RESTP]  = "RESTP",
  [OPC_RSTAT]  = "RSTAT",
  [OPC_QNN]    = "QNN",
  [OPC_RQNP]   = "RQNP",
  [OPC_RQMN]   = "RQMN",

  -- 20–3F - 1 data byte packets:
  [OPC_KLOC]   = "KLOC",
  [OPC_QLOC]   = "QLOC",
  [OPC_DKEEP]  = "DKEEP",
  [OPC_DBG1]   = "DBG1",
  [OPC_EXTC]   = "EXTC",

  -- 40–5F - 2 data byte packets:
  [OPC_RLOC]   = "RLOC",
  [OPC_QCON]   = "QCON",
  [OPC_SNN]    = "SNN",
  [OPC_ALOC]   = "ALOC",
  [OPC_STMOD]  = "STMOD",
  [OPC_PCON]   = "PCON",
  [OPC_KCON]   = "KCON",
  [OPC_DSPD]   = "DSPD",
  [OPC_DFLG]   = "DFLG",
  [OPC_DFNON]  = "DFNON",
  [OPC_DFNOF]  = "DFNOF",
  [OPC_SSTAT]  = "SSTAT",
  [OPC_NNRSM]  = "NNRSM",
  [OPC_RQNN]   = "RQNN",
  [OPC_NNREL]  = "NNREL",
  [OPC_NNACK]  = "NNACK",
  [OPC_NNLRN]  = "NNLRN",
  [OPC_NNULN]  = "NNULN",
  [OPC_NNCLR]  = "NNCLR",
  [OPC_NNEVN]  = "NNEVN",
  [OPC_NERD]   = "NERD",
  [OPC_RQEVN]  = "RQEVN",
  [OPC_WRACK]  = "WRACK",
  [OPC_RQDAT]  = "RQDAT",
  [OPC_RQDDS]  = "RQDDS",
  [OPC_BOOTM]  = "BOOTM",
  [OPC_ENUM]   = "ENUM",
  [OPC_NNRST]  = "NNRST",
  [OPC_EXTC1]  = "EXTC1",

  -- 60-7F - 3 data byte packets:
  [OPC_DFUN]   = "DFUN",
  [OPC_GLOC]   = "GLOC",
  [OPC_ERR]    = "ERR",
  [OPC_CMDERR] = "CMDERR",
  [OPC_EVNLF]  = "EVNLF",
  [OPC_NVRD]   = "NVRD",
  [OPC_NENRD]  = "NENRD",
  [OPC_RQNPN]  = "RQNPN",
  [OPC_NUMEV]  = "NUMEV",
  [OPC_CANID]  = "CANID",
  [OPC_EXTC2]  = "EXTC2",

  -- 80-9F - 4 data byte packets:
  [OPC_RDCC3]  = "RDCC3",
  [OPC_WCVO]   = "WCVO",
  [OPC_WCVB]   = "WCVB",
  [OPC_QCVS]   = "QCVS",
  [OPC_PCVS]   = "PCVS",
  [OPC_ACON]   = "ACON",
  [OPC_ACOF]   = "ACOF",
  [OPC_AREQ]   = "AREQ",
  [OPC_ARON]   = "ARON",
  [OPC_AROF]   = "AROF",
  [OPC_EVULN]  = "EVULN",
  [OPC_NVSET]  = "NVSET",
  [OPC_NVANS]  = "NVANS",
  [OPC_ASON]   = "ASON",
  [OPC_ASOF]   = "ASOF",
  [OPC_ASRQ]   = "ASRQ",
  [OPC_PARAN]  = "PARAN",
  [OPC_REVAL]  = "REVAL",
  [OPC_ARSON]  = "ARSON",
  [OPC_ARSOF]  = "ARSOF",
  [OPC_EXTC3]  = "EXTC3",

  -- A0-BF - 5 data byte packets:
  [OPC_RDCC4]  = "RDCC4",
  [OPC_WCVS]   = "WCVS",
  [OPC_ACON1]  = "ACON1",
  [OPC_ACOF1]  = "ACOF1",
  [OPC_REQEV]  = "REQEV",
  [OPC_ARON1]  = "ARON1",
  [OPC_AROF1]  = "AROF1",
  [OPC_NEVAL]  = "NEVAL",
  [OPC_PNN]    = "PNN",
  [OPC_ASON1]  = "ASON1",
  [OPC_ASOF1]  = "ASOF1",
  [OPC_ARSON1] = "ARSON1",
  [OPC_ARSOF1] = "ARSOF1",
  [OPC_EXTC4]  = "EXTC4",

  -- C0-DF - 6 data byte packets:
  [OPC_RDCC5]  = "RDCC5",
  [OPC_WCVOA]  = "WCVOA",
  [OPC_CABDAT] = "CABDAT",
  [OPC_FCLK]   = "FCLK",
  [OPC_ACON2]  = "ACON2",
  [OPC_ACOF2]  = "ACOF2",
  [OPC_EVLRN]  = "EVLRN",
  [OPC_EVANS]  = "EVANS",
  [OPC_ARON2]  = "ARON2",
  [OPC_AROF2]  = "AROF2",
  [OPC_ASON2]  = "ASON2",
  [OPC_ASOF2]  = "ASOF2",
  [OPC_ARSON2] = "ARSON2",
  [OPC_ARSOF2] = "ARSOF2",
  [OPC_EXTC5]  = "EXTC5",

  -- E0-FF - 7 data byte packets:
  [OPC_RDCC6]  = "RDCC6",
  [OPC_PLOC]   = "PLOC",
  [OPC_NAME]   = "NAME",
  [OPC_STAT]   = "STAT",
  [OPC_PARAMS] = "PARAMS",
  [OPC_ACON3]  = "ACON3",
  [OPC_ACOF3]  = "ACOF3",
  [OPC_ENRSP]  = "ENRSP",
  [OPC_ARON3]  = "ARON3",
  [OPC_AROF3]  = "AROF3",
  [OPC_EVLRNI] = "EVLRNI",
  [OPC_ACDAT]  = "ACDAT",
  [OPC_ARDAT]  = "ARDAT",
  [OPC_ASON3]  = "ASON3",
  [OPC_ASOF3]  = "ASOF3",
  [OPC_DDES]   = "DDES",
  [OPC_DDRS]   = "DDRS",
  [OPC_ARSON3] = "ARSON3",
  [OPC_ARSOF3] = "ARSOF3",
  [OPC_EXTC6]  = "EXTC6",
}

local pf_id       = ProtoField.uint16("cbus.id", "ID", base.HEX)
local pf_id_major = ProtoField.uint8("cbus.id.major", "Major Priority", base.DEC, major_priorities)
local pf_id_minor = ProtoField.uint8("cbus.id.minor", "Minor Priority", base.DEC, minor_priorities)
local pf_id_can   = ProtoField.uint8("cbus.id.canid", "CAN ID", base.DEC)
local pf_opcode   = ProtoField.uint8("cbus.opcode", "OpCode", base.HEX, opcodes)
local pf_data     = ProtoField.string("cbus.data", "Data")

cbus.fields = {pf_id, pf_id_major, pf_id_minor, pf_id_can, pf_opcode, pf_data}
cbus.prefs.port = Pref.uint("Port number", default_settings.port, "The TCP port number for CBUS/VLCB protocol")

function cbus.prefs_changed()
  if default_settings.port ~= cbus.prefs.port then
    if default_settings.port ~= 0 then
      DissectorTable.get("tcp.port"):remove(default_settings.port, cbus)
    end
    default_settings.port = cbus.prefs.port
    if default_settings.port ~= 0 then
      DissectorTable.get("tcp.port"):add(default_settings.port, cbus)
    end
  end
end

function cbus.dissector(tvbuf, pktinfo, root)
  pktinfo.cols.protocol:set("CBUS/VLCB")

  while tvbuf:reported_length_remaining() > 0 do
    local payload = tvbuf():string()
    local frame, id, opcode, data = payload:match("(:S(%x%x%x%x)N(%x%x)(%x*);)")

    if not id then
      return
    end

    id = bit.rshift(tonumber(id, 16), 5)
    local major_priority = bit.band(bit.rshift(id, 9), 0x03)
    local minor_priority = bit.band(bit.rshift(id, 7), 0x03)
    local can_id = bit.band(id, 0x7F)
    opcode = tonumber(opcode, 16)

    local tree = root:add(cbus, tvbuf(), "CBUS/VLCB")
    local id_tree = tree:add(pf_id, tvbuf(2, 4), id)
    id_tree:append_text(string.format(" (MjPri:%d, MinPri:%d, CAN ID:%d)", major_priority, minor_priority, can_id))
    id_tree:add(pf_id_major, tvbuf(2, 1), major_priority)
    id_tree:add(pf_id_minor, tvbuf(2, 1), minor_priority)
    id_tree:add(pf_id_can,   tvbuf(3, 2), can_id)
    tree:add(pf_opcode, tvbuf(7, 2), opcode)
    tree:add(pf_data, tvbuf(9, #data),  data)

    tvbuf = tvbuf:range(#frame):tvb()
  end
end

DissectorTable.get("tcp.port"):add(default_settings.port, cbus)
