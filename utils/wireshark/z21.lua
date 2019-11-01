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

local z21 = Proto("z21", "Z21 protocol")

local pf_data_len = ProtoField.uint16("z21.data_len", "DataLen", base.HEX)
local pf_header = ProtoField.uint16("z21.data_len", "DataLen", base.HEX)
local pf_data = ProtoField.bytes("z21.data", "Data")


--[[
local pf_type = ProtoField.uint8("z21.flags.type", "Type", base.DEC, flag_types, 0xc0)
local pf_errorcode = ProtoField.uint8("z21.error_code", "Error code", base.DEC, nil, 0x3f)
local pf_requestid = ProtoField.uint16("z21.request_id", "Request id")
local pf_datasize = ProtoField.uint32("z21.data_size", "Data size")
local pf_data = ProtoField.bytes("z21.data", "Data")

z21.fields = {
  pf_command,
  pf_type,
  pf_errorcode,
  pf_requestid,
  pf_datasize,
  pf_data,
}
]]
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
  --[[
  while tvbuf:reported_length_remaining() >= 4 do
    local data_size = tvbuf:range(4, 4)
    local tree = root:add(z21, tvbuf:range(0, TRAINTASTIC_HDR_LEN + data_size:le_uint()))

    tree:add_le(pf_command, tvbuf:range(0, 1))
    local flags = tvbuf:range(1, 1)
    tree:add(pf_type, flags)
    tree:add(pf_errorcode, flags)
    tree:add_le(pf_requestid, tvbuf:range(2, 2))
    tree:add_le(pf_datasize, data_size)

    tvbuf = tvbuf:range(TRAINTASTIC_HDR_LEN + data_size:le_uint()):tvb()
  end
  ]]
end

DissectorTable.get("udp.port"):add(default_settings.port, z21)
