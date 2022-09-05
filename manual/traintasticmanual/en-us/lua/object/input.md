# Input {#lua-object-input}


## Properties

### `id`
Object id, unique within the world.

### `name`
Input name.

### `value`
Input value, an [`enum.tristate`](../library/enum/tristate.md) value.

Note: The input value can be `enum.tristate.UNDEFINED`, especially when just connected to the hardware.


## Methods

*None*.


## Events

### `on_value_changed`
Fired when the input value changes.

Handler: `function (value, input, user_data)`
- *value* - `true` if the input value changed to high, `false` if the input value changed to low;
- *input* - the input object;
- *user_data* - user data that was set or `nil` if no user data was set during connect.

Example: [Direction control using an input](../../luaexamples/directioncontrolusinginput.md).
