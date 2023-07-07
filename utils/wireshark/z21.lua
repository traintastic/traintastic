--[[

Z21 protocol dissector for WireShark

Copyright (c) 2019 Reinder Feenstra

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
  port = 21105
}

local LAN_GET_SERIAL_NUMBER = 0x10
local LAN_GET_CODE = 0x18
local LAN_GET_HWINFO = 0x1a
local LAN_LOGOFF = 0x30
local LAN_X = 0x40
local LAN_SET_BROADCASTFLAGS = 0x50
local LAN_GET_BROADCASTFLAGS = 0x51
local LAN_GET_LOCOMODE = 0x60
local LAN_SET_LOCOMODE = 0x61
local LAN_GET_TURNOUTMODE = 0x70
local LAN_SET_TURNOUTMODE = 0x71
local LAN_RMBUS_GETDATA = 0x81
local LAN_RMBUS_PROGRAMMODULE = 0x82
LAN_SYSTEMSTATE_DATACHANGED = 0x84
local LAN_SYSTEMSTATE_GETDATA = 0x85
local LAN_RAILCOM_GETDATA = 0x89
local LAN_CAN_DETECTOR = 0xc4

-- LAN_LOCONET_DETECTOR
local LAN_LOCONET_Z21_RX = 0xa0
local LAN_LOCONET_Z21_TX = 0xa1
local LAN_LOCONET_FROM_LAN = 0xa2
local LAN_LOCONET_DISPATCH_ADDR = 0xa3
local LAN_LOCONET_DETECTOR = 0xa4

local header_values =
{
  [LAN_GET_SERIAL_NUMBER] = "LAN_GET_SERIAL_NUMBER",
  [LAN_GET_CODE] = "LAN_GET_CODE",
  [LAN_GET_HWINFO] = "LAN_GET_HWINFO",
  [LAN_LOGOFF] = "LAN_LOGOFF",
  [LAN_X] = "LAN_X",
  [LAN_SET_BROADCASTFLAGS] = "LAN_SET_BROADCASTFLAGS",
  [LAN_GET_BROADCASTFLAGS] = "LAN_GET_BROADCASTFLAGS",
  [LAN_GET_LOCOMODE] = "LAN_GET_LOCOMODE",
  [LAN_SET_LOCOMODE] = "LAN_SET_LOCOMODE",
  [LAN_GET_TURNOUTMODE] = "LAN_GET_TURNOUTMODE",
  [LAN_SET_TURNOUTMODE] = "LAN_SET_TURNOUTMODE",
  [LAN_RMBUS_GETDATA] = "LAN_RMBUS_GETDATA",
  [LAN_RMBUS_PROGRAMMODULE] = "LAN_RMBUS_PROGRAMMODULE",
  [LAN_SYSTEMSTATE_DATACHANGED] = "LAN_SYSTEMSTATE_DATACHANGED",
  [LAN_SYSTEMSTATE_GETDATA] = "LAN_SYSTEMSTATE_GETDATA",
  [LAN_RAILCOM_GETDATA] = "LAN_RAILCOM_GETDATA",
  [LAN_CAN_DETECTOR] = "LAN_CAN_DETECTOR",

  [LAN_LOCONET_Z21_RX] = "LAN_LOCONET_Z21_RX",
  [LAN_LOCONET_Z21_TX] = "LAN_LOCONET_Z21_TX",
  [LAN_LOCONET_FROM_LAN] = "LAN_LOCONET_FROM_LAN",
  [LAN_LOCONET_DISPATCH_ADDR] = "LAN_LOCONET_DISPATCH_ADDR",
  [LAN_LOCONET_DETECTOR] = "LAN_LOCONET_DETECTOR",
}

local LAN_X_SET_LOCO = 0xe4
local LAN_X_SET_LOCO_FUNCTION = 0xf8 -- db0
local LAN_X_LOCO_INFO = 0xef


local LAN_X_GET_VERSION = 0x21
local LAN_X_GET_STATUS = 0x21
local LAN_X_SET_TRACK_POWER_OFF = 0x0
local LAN_X_SET_TRACK_POWER_ON = 0x0
local LAN_X_BC_TRACK_POWER_OFF = 0x0
local LAN_X_BC_TRACK_POWER_ON = 0x0
local LAN_X_BC_PROGRAMMING_MODE = 0x0
local LAN_X_BC_TRACK_SHORT_CIRCUIT = 0x0
local LAN_X_UNKNOWN_COMMAND = 0x61
local LAN_X_STATUS_CHANGED = 0x62
local LAN_X_SET_STOP = 0x80
local LAN_X_BC_STOPPED = 0x81
local LAN_X_GET_FIRMWARE_VERSION = 0xF1

local lanx_header_values =
{
  [LAN_X_SET_LOCO] = "LAN_X_SET_LOCO",
  [LAN_X_LOCO_INFO] = "LAN_X_LOCO_INFO",
  [LAN_X_SET_STOP] = "LAN_X_SET_STOP",
  [LAN_X_BC_STOPPED] = "LAN_X_BC_STOPPED",
  [LAN_X_UNKNOWN_COMMAND] = "LAN_X_UNKNOWN_COMMAND"
}

local direction_string_table =
{
  [1] = "Forward",
  [2] = "Reverse"
}

local func_value_string_table =
{
  [0] = "OFF",
  [1] = "ON",
  [2] = "TOGGLE",
  [3] = "INVALID"
}

local z21 = Proto("z21", "Z21 protocol")

local pf_data_len = ProtoField.uint16("z21.data_len", "DataLen", base.HEX)
local pf_header = ProtoField.uint16("z21.header", "Header", base.HEX, header_values)
local pf_data = ProtoField.bytes("z21.data", "Data")
local pf_lanx_header = ProtoField.uint8("z21.xheader", "X-Header", base.HEX, lanx_header_values)

local pf_loco_address = ProtoField.uint16("z21.loco_address", "Loco Address", base.DEC)
local pf_loco_busy = ProtoField.bool("z21.loco_busy", "Loco Busy")
local pf_dcc_steps = ProtoField.uint8("z21.dcc_steps", "DCC Steps", base.DEC)
local pf_loco_direction = ProtoField.bool("z21.loco_direction", "Loco Direction", base.BIN, direction_string_table)
local pf_loco_speed = ProtoField.uint8("z21.loco_speed", "Loco Speed", base.DEC)
local pf_loco_estop = ProtoField.bool("z21.loco_estop", "Emergency Stop")
local pf_loco_raw_speed = ProtoField.uint8("z21.loco_raw_speed", "Loco Raw Speed", base.HEX)

local pf_loco_func_number = ProtoField.uint8("z21.loco_func_number", "Function No", base.DEC)
local pf_loco_func_value = ProtoField.uint8("z21.loco_func_value", "Function Val", base.BIN, func_value_string_table)

local pf_cs_state = ProtoField.uint8("z21.cs_state", "CS State", base.HEX)
local pf_cs_state_ex = ProtoField.uint8("z21.cs_state_ex", "CS State Ex", base.HEX)
local pf_cs_capabilities = ProtoField.uint8("z21.cs_capabilities", "Capabilities", base.HEX)
local pf_cs_estop = ProtoField.bool("z21.cs_estop", "Em. Stop")
local pf_cs_trackvoltageoff = ProtoField.bool("z21.cs_trackvoltageoff", "Track Voltage Off")


--[[
local pf_type = ProtoField.uint8("z21.flags.type", "Type", base.DEC, flag_types, 0xc0)
local pf_errorcode = ProtoField.uint8("z21.error_code", "Error code", base.DEC, nil, 0x3f)
local pf_requestid = ProtoField.uint16("z21.request_id", "Request id")
local pf_datasize = ProtoField.uint32("z21.data_size", "Data size")
local pf_data = ProtoField.bytes("z21.data", "Data")
]]
z21.fields = {
  pf_data_len,
  pf_header,
  pf_data,
  pf_lanx_header,
  pf_loco_address,
  pf_loco_busy,
  pf_dcc_steps,
  pf_loco_direction,
  pf_loco_speed,
  pf_loco_estop,
  pf_loco_raw_speed,
  pf_loco_func_number,
  pf_loco_func_value,
  pf_cs_state,
  pf_cs_state_ex,
  pf_cs_capabilities,
  pf_cs_estop,
  pf_cs_trackvoltageoff
}

z21.prefs.port = Pref.uint("Port number", default_settings.port, "The UDP port number for Z21 protocol")

function parseAddress(loco_addrMSB, loco_addrLSB)
  local loco_addr = bit.lshift(bit.band(loco_addrMSB, 0x3F), 8) + loco_addrLSB
  return loco_addr
end

function parseSpeedStep(max_steps, step)
  local real_step = step
  if (max_steps == 14 or max_steps == 128) and real_step >= 1 then
    real_step = real_step - 1 -- Skip E-Stop step
  else
    local a = bit.lshift(bit.band(step, 0x0f), 1)
    local b = bit.rshift(bit.band(step, 0x10), 4)
    real_step = bit.bor(a, b)
    if real_step >= 3 then
      real_step = real_step - 1 -- Skip 2 values of E-Stop and another value for Stop
    end
  end
  return real_step
end

function parseEStop(max_steps, step)
  local estop = false
  if step == 0x01 or (max_steps == 28 and step == 0x01) then
    estop = true
  end
  return estop
end

function parseSetLoco(tree, tvbuf, xitem)
  local loco_addrMSB = tvbuf:range(6, 1):uint()
  local loco_addrLSB = tvbuf:range(7, 1):uint()
  local loco_addr = parseAddress(loco_addrMSB, loco_addrLSB)
  tree:add(pf_loco_address, loco_addr)

  local db0 = tvbuf:range(5, 1):uint()
  local db3 = tvbuf:range(8, 1):uint()

  if bit.band(db0, 0xF0) == 0x10 then
    xitem:set_text("LAN_X_SET_LOCO_DRIVE")

    local max_steps = 128
    if db0 == 0x12 then
      max_steps = 28
    elseif db0 == 0x10 then
      max_steps = 14
    end
    tree:add(pf_dcc_steps, max_steps)

    local dir = bit.band(db3, 0x80)
    local speed = bit.band(db3, 0x7f)
    local estop = parseEStop(max_steps, speed)
    local real_step = parseSpeedStep(max_steps, speed)

    tree:add(pf_loco_direction, dir)
    tree:add(pf_loco_speed, real_step)
    tree:add(pf_loco_estop, estop)
    tree:add(pf_loco_raw_speed, speed)

  elseif db0 == LAN_X_SET_LOCO_FUNCTION then
    xitem:set_text("LAN_X_SET_LOCO_FUNCTION")

    local val = bit.rshift(db3, 6)
    local func = bit.band(db3, 0x3F)

    tree:add(pf_loco_func_number, func)
    tree:add(pf_loco_func_value, val)
  end
end

function parseLocoInfo(tree, tvbuf)
  local loco_addrMSB = tvbuf:range(5, 1):uint()
  local loco_addrLSB = tvbuf:range(6, 1):uint()
  local loco_addr = parseAddress(loco_addrMSB, loco_addrLSB)
  tree:add(pf_loco_address, loco_addr)

  local db2 = tvbuf:range(7, 1):uint()
  local busy = bit.band(db2, 0x08)
  tree:add(pf_loco_busy, busy)

  local stepsbit = bit.band(db2, 0x07)
  local max_steps = 128
  if stepsbit == 1 then
    max_steps = 14
  elseif stepsbit == 2 then
    max_steps = 28
  end
  tree:add(pf_dcc_steps, max_steps)

  local db3 = tvbuf:range(8, 1):uint()

  local dir = bit.band(db3, 0x80)
  local speed = bit.band(db3, 0x7f)
  local estop = parseEStop(max_steps, speed)
  local real_step = parseSpeedStep(max_steps, speed)

  tree:add(pf_loco_direction, dir)
  tree:add(pf_loco_speed, real_step)
  tree:add(pf_loco_estop, estop)
  tree:add(pf_loco_raw_speed, speed)
end

function parseStatusChanged(tree, tvbuf)
  local is_x_status = (tvbuf:range(2, 2):le_uint() == LAN_X)
  local offset = 4
  if is_x_status then
    offset = 6
  end

  local cs_state = tvbuf:range(offset + 12, 1)
  tree:add(pf_cs_state, cs_state)

  local isStop = bit.band(cs_state:uint(), 0x01)
  tree:add(pf_cs_estop, isStop)

  local trk_volt_off = bit.band(cs_state:uint(), 0x02)
  tree:add(pf_cs_trackvoltageoff, trk_volt_off)

  if not is_x_status then
    local cs_state_ex = tvbuf:range(offset + 13, 1)
    tree:add(pf_cs_state_ex, cs_state_ex)

    local cs_capabilities = tvbuf:range(offset + 15, 1)
    tree:add(pf_cs_capabilities, cs_capabilities)
  end
end

function z21.prefs_changed()
  if default_settings.port ~= z21.prefs.port then
    if default_settings.port ~= 0 then
      DissectorTable.get("udp.port"):remove(default_settings.port, z21)
    end
    default_settings.port = z21.prefs.port
    if default_settings.port ~= 0 then
      DissectorTable.get("udp.port"):add(default_settings.port, z21)
    end
  end
end

function z21.dissector(tvbuf, pktinfo, root)
  pktinfo.cols.protocol:set("Z21")

  while tvbuf:reported_length_remaining() >= 4 do
    local data_len = tvbuf:range(0, 2)
    local header = tvbuf:range(2, 2)
    local tree = root:add(z21, tvbuf:range(0, data_len:le_uint()))
    tree:add_le(pf_data_len, data_len)
    tree:add_le(pf_header, header)
    if data_len:le_uint() > 4 then
      tree:add(pf_data, tvbuf:range(4, data_len:le_uint() - 4))
    end

    if header:le_uint() == LAN_SYSTEMSTATE_DATACHANGED then
      parseStatusChanged(tree, tvbuf)
    end

    if header:le_uint() == LAN_X then
      local xheader = tvbuf:range(4, 1)
      local xitem = tree:add(pf_lanx_header, xheader)

      if xheader:le_uint() == LAN_X_SET_LOCO then
        parseSetLoco(tree, tvbuf, xitem)
      elseif xheader:le_uint() == LAN_X_LOCO_INFO then
        parseLocoInfo(tree, tvbuf)
      elseif xheader:le_uint() == LAN_X_STATUS_CHANGED then
        parseStatusChanged(tree, tvbuf)
      elseif xheader:le_uint() == 0x21 then
        local db0 = tvbuf:range(4, 1):uint()
        if db0 == 0x80 then
          xitem:set_text("LAN_X_SET_TRACK_POWER_OFF")
        elseif db0 == 0x81 then
          xitem:set_text("LAN_X_SET_TRACK_POWER_ON")
        end
      elseif xheader:le_uint() == 0x61 then
        local db0 = tvbuf:range(4, 1):uint()
        if db0 == 0x00 then
          xitem:set_text("LAN_X_BC_TRACK_POWER_OFF")
        elseif db0 == 0x01 then
          xitem:set_text("LAN_X_BC_TRACK_POWER_ON")
        elseif db0 == 0x02 then
          xitem:set_text("LAN_X_SET_PROGRAMMING_MODE")
        elseif db0 == 0x08 then
          xitem:set_text("LAN_X_BC_TRACK_SHORT_CIRCUIT")
        end
      end
    end

    tvbuf = tvbuf:range(data_len:le_uint()):tvb()
  end
end

DissectorTable.get("udp.port"):add(default_settings.port, z21)
