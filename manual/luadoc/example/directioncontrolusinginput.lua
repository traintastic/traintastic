world.get_object('input_1').on_value_changed(
  function (value)
    -- get the direction tile to control
    local direction_control = world.get_object('direction_1')
    -- state state based on input value
    if value then
      direction_control.set_state(enum.direction_control_state.B_TO_A)
    else
      direction_control.set_state(enum.direction_control_state.A_TO_B)
    end
  end)
