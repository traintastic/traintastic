# Input {#lua-object-input}


## Properties

### `id`
Object id, unique within the world.

### `name`
Input name.

### `value`
Input value, an `enum.tristate` value.

Note: The input value can be `enum.tristate.UNDEFINED`, especially when just connected to the hardware.


## Methods

### `get_object(id)`
Get object with *id*, if it exists it returns the object, else it returns `nil`.

### `power_off()`
Stop all vehicles and power off, identical to pressing power off in the Traintastic client application.

### `stop()`
Stop all vehicles, identical to pressing stop in the Traintastic client application.


## Events

### `on_value_changed`
Fired when the input value changes.

Handler: `function (value, input, user_data)`
- *value* - `true` if the input value changed to high, `false` if the input value changed to low;
- *input* - the input object;
- *user_data* - user data that was set or `nil` if no user data was set during connect.
