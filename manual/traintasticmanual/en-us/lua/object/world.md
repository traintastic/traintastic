# World {#lua-object-world}


## Properties

### `boards` $badge:since:v0.1$
[List](objectlist.md) of all boards.

### `name` $badge:since:v0.1$
World name.

### `rail_vehicles` $badge:since:v0.1$
[List](objectlist.md) of all rail vehicles.

### `scale` $badge:since:v0.1$
World scale, an [`enum.world_scale`](../library/enum/worldscale.md) value.

### `scale_ratio` $badge:since:v0.1$
World scale ratio in 1:*value*, e.g. for H0 it returns 87.

### `state` $badge:since:v0.1$
World state, a [`set.world_state`](../library/set/worldstate.md) value.

### `trains` $badge:since:v0.1$
[List](objectlist.md) of all trains.

### `uuid` $badge:since:v0.1$
World UUID (Universal Unique Identifier), every world that is created has its own unique UUID.


## Methods

### `get_object(id)` $badge:since:v0.1$
Get object with *id*, if it exists it returns the object, else it returns `nil`.

### `power_off()` $badge:since:v0.1$
Stop all vehicles and power off, identical to pressing power off in the Traintastic client application.

### `stop()` $badge:since:v0.1$
Stop all vehicles, identical to pressing stop in the Traintastic client application.


## Events

### `on_event` $badge:since:v0.1$
Fired when the world state changes,
e.g. when pressing the stop button in the Traintastic client application or calling `world.stop()`.

Handler: `function (state, event, world, user_data)`
- *state* - a [`set.world_state`](../library/set/worldstate.md) value;
- *event* - an [`enum.world_event`](../library/enum/worldevent.md) value;
- *world* - the world object;
- *user_data* - user data that was set or `nil` if no user data was set during connect.

Example: [Control output on world run/stop](../../luaexamples/controloutputworldrunstop.md).
