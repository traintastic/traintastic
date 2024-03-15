local loconet = world.get_object('loconet_1') -- LocoNet interface object

function dcc_ext(address, aspect)
  --[[
  Build DCCext command

  Format is: [10AAAAAA] [0AAA0AA1] [DDDDDDDD] (3 bytes)
  where:
    A = address bit
    D = data/aspect bit

  See RCN 213: http://normen.railcommunity.de/RCN-213.pdf
  ]]

  address = address + 3 -- User address 1 is address 4 in the command

  -- Return a table with three items, one for each command byte:
  return {
    0x80 | ((address >> 2) & 0x3F),
    ((~address >> 4) & 0x70) | ((address << 1) & 0x06) | 0x01, -- Note: Address bits 8-10 are inverted!
    aspect
  }
end

function set_aspect(signal, aspect, address)
  -- Signal aspect changed handler, this function is called once every time the signals aspect changes.
  local value = 0

  -- Values may vary depending on used decoder, zero is always STOP, see RCN 213.
  -- These value match a Dutch dwarf signal connected to a YaMoRC YD8116.
  if aspect == enum.signal_aspect.STOP then
    value = 0
  elseif aspect == enum.signal_aspect.PROCEED_REDUCED_SPEED then
    value = 1
  elseif aspect == enum.signal_aspect.PROCEED then
    value = 16
  end

  -- Send DCC command to the track using LocoNet OPC_IMM_PACKET:
  loconet.imm_packet(dcc_ext(address, value))
end

-- Set the handler, the address (1) is set as user data, so the function can easily be used for multiple signals
world.get_object('signal_1').on_aspect_changed(set_aspect, 1)
