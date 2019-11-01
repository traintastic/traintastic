--[[

Traintastic protocol dissector for WireShark

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
  port = 5740
}

local traintastic = Proto("traintastic", "Traintastic protocol")

local commands = {
  [0] = "Invalid",
  [1] = "Ping",
  [2] = "Login",
  [3] = "NewSession",
  -- [4] = "RestoreSession",
  -- [5] = "NewWorld",
  -- [6] = "GetWorldList",
  -- [7] = "LoadWorld",
  -- [8] = "SaveWorld",
  -- [9] = "ImportWorld",
  -- [10] = "ExportWorld",
  -- [11] = "CreateObject",
  -- [12] = "DeleteObject",
  -- [13] = "IsObject",
  [14] = "GetObject",
  -- [15] = "ReleaseObject",
  -- [16] = "SetProperty",
  -- [17] = "PropertyChanged",
  [18] = "GetView",

  [255] = "Discover",
}

local flag_types = {
  [1] = "Request",
  [2] = "Response",
  [3] = "Event",
}

local pf_command = ProtoField.uint8("traintastic.command", "Command", base.HEX, commands)
local pf_type = ProtoField.uint8("traintastic.flags.type", "Type", base.DEC, flag_types, 0xc0)
local pf_errorcode = ProtoField.uint8("traintastic.error_code", "Error code", base.DEC, nil, 0x3f)
local pf_requestid = ProtoField.uint16("traintastic.request_id", "Request id")
local pf_datasize = ProtoField.uint32("traintastic.data_size", "Data size")
local pf_data = ProtoField.bytes("traintastic.data", "Data")

traintastic.fields = {
  pf_command,
  pf_type,
  pf_errorcode,
  pf_requestid,
  pf_datasize,
  pf_data,
}

traintastic.prefs.port = Pref.uint("Port number", default_settings.port, "The TCP/UDP port number for Traintastic protocol")

function traintastic.prefs_changed()
  if default_settings.port ~= traintastic.prefs.port then
    if default_settings.port ~= 0 then
      DissectorTable.get("tcp.port"):remove(default_settings.port, traintastic)
    end
    default_settings.port = traintastic.prefs.port
    if default_settings.port ~= 0 then
      DissectorTable.get("tcp.port"):add(default_settings.port, traintastic)
    end
  end
end

local TRAINTASTIC_HDR_LEN = 8

function traintastic.dissector(tvbuf, pktinfo, root)
  pktinfo.cols.protocol:set("Traintastic")

  while tvbuf:reported_length_remaining() >= TRAINTASTIC_HDR_LEN do
    local data_size = tvbuf:range(4, 4)
    local tree = root:add(traintastic, tvbuf:range(0, TRAINTASTIC_HDR_LEN + data_size:le_uint()))

    tree:add_le(pf_command, tvbuf:range(0, 1))
    local flags = tvbuf:range(1, 1)
    tree:add(pf_type, flags)
    tree:add(pf_errorcode, flags)
    tree:add_le(pf_requestid, tvbuf:range(2, 2))
    tree:add_le(pf_datasize, data_size)

    tvbuf = tvbuf:range(TRAINTASTIC_HDR_LEN + data_size:le_uint()):tvb()
  end
end

DissectorTable.get("tcp.port"):add(default_settings.port, traintastic)
DissectorTable.get("udp.port"):add(default_settings.port, traintastic)
