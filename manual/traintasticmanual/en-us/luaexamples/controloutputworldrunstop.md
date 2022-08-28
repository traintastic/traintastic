# Control output on world run/stop $badge:since:v0.2$ {#lua-examples-control-output-on-world-run-stop}

This examples monitors the world state and controls an output based on the *run* flag. If the world in in *run* state the output is off, if the world is stopped the output is on, e.g. for controller a red lamp.

```lua
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
world.on_event = world_event

-- call event handler once at script start to set output to current world state
world_event(world.state)
```

The event handler above is a bit verbose for clarity, it can also be written more compact as:

```lua
function world_event(state)
  world.get_object('output_1').set_value(not world.state.contains(set.world_state.RUN))
end
```
