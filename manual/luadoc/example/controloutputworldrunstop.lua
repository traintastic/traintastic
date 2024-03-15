-- get the output to control
output = world.get_object('loconet_1').get_output(enum.output_channel.ACCESSORY, 1)

-- world event handler
function world_event(state)
  -- check if the RUN flag is set in the world state
  if world.state.contains(set.world_state.RUN) then
    output.set_value(enum.output_pair_value.SECOND)
  else
    output.set_value(enum.output_pair_value.FIRST)
  end
end

-- register the event handler
world.on_event(world_event)
