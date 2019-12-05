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
local LAN_SYSTEMSTATE_GETDATA = 0x85
local LAN_RAILCOM_GETDATA = 0x89
local LAN_LOCONET_FROM_LAN = 0xa2
local LAN_LOCONET_DISPATCH_ADDR = 0xa3
local LAN_LOCONET_DETECTOR = 0xa4
local LAN_CAN_DETECTOR = 0xc4

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
  [LAN_SYSTEMSTATE_GETDATA] = "LAN_SYSTEMSTATE_GETDATA",
  [LAN_RAILCOM_GETDATA] = "LAN_RAILCOM_GETDATA",
  [LAN_LOCONET_FROM_LAN] = "LAN_LOCONET_FROM_LAN",
  [LAN_LOCONET_DISPATCH_ADDR] = "LAN_LOCONET_DISPATCH_ADDR",
  [LAN_LOCONET_DETECTOR] = "LAN_LOCONET_DETECTOR",
  [LAN_CAN_DETECTOR] = "LAN_CAN_DETECTOR",
}

local z21 = Proto("z21", "Z21 protocol")

local pf_data_len = ProtoField.uint16("z21.data_len", "DataLen", base.HEX)
local pf_header = ProtoField.uint16("z21.data_len", "Header", base.HEX, header_values)
local pf_data = ProtoField.bytes("z21.data", "Data")


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
}

z21.prefs.port = Pref.uint("Port number", default_settings.port, "The UDP port number for Z21 protocol")

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
    local tree = root:add(z21, tvbuf:range(0, data_len:le_uint()))
    tree:add_le(pf_data_len, data_len)
    tree:add_le(pf_header, tvbuf:range(2, 2))
    if data_len:le_uint() > 4 then
      tree:add(pf_data, tvbuf:range(4, data_len:le_uint() - 4))
    end

    tvbuf = tvbuf:range(data_len:le_uint()):tvb()
  end
end

DissectorTable.get("udp.port"):add(default_settings.port, z21)
