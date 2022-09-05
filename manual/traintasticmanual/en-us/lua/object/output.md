# Output {#lua-object-output}


## Properties

### `id`
Object id, unique within the world.

### `name`
Output name.

### `value`
Output value, an [`enum.tristate`](../library/enum/tristate.md) value.

Note: The output value can be `enum.tristate.UNDEFINED`, especially when just connected to the hardware.


## Methods

### `set_value(value)`
Set the output value to `true` or `false`. The method returns `true` if the command was succesfully sent to the hardware, `false` otherwise.

Note: Reading `value` after calling `set_value()` may still return the old value, `value` is updated after the hardware comfirmed the newly set value.


## Events

### `on_value_changed`
Fired when the output value changes.

Handler: `function (value, output, user_data)`
- *value* - `true` if the output value changed to high, `false` if the output value changed to low;
- *output* - the output object;
- *user_data* - user data that was set or `nil` if no user data was set during connect.
