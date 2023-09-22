-- world event handler
function world_event(state)
  -- get the output to control
  local output = world.get_object('output_1')

  -- check if the RUN flag is set in the world state
  if world.state.contains(set.world_state.RUN) then
    -- turn output off
    output.set_value(false)
  else
    -- turn output on
    output.set_value(true)
  end
end

-- register the event handler
world.on_event(world_event)

-- call event handler once at script start to set output to current world state
world_event(world.state)
