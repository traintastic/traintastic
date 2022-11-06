# Clock $badge:since:v0.2$ {#lua-object-clock}

The clock object represents the time of the world. There is only one clock object, it can be accessed using `world.clock`.

## Properties

### `freeze`
`True` if clock is frozen, the clock does not tick.

Note: If the clock is not frozen but the world is in *Stop* state the clock does not tick. To check if the clock is running check [`running`](#lua-object-clock-running)

### `hour`
Hour, 0...23.

### `minute`
Minute, 0...60.

### `multiplier`
Fast clock multiplier rate. That is, one minute in real time represents *multiplier* minutes of world time.

### `running` {#lua-object-clock-running}
`True` if clock is running, `False` if the clock is frozen or the world is in *Stop* state.

Note: The clock only runs if it is not frozen and the world is in *Run* state.

### `time`
Current time in ticks since midnight, each tick is one fast clock minute.


## Methods

*None.*


## Events

### `on_resume`
Fired when the clock resumes.

Handler: `function (time, multiplier, clock, user_data)`
- *time* - current time in ticks since midnight, each tick is one fast clock minute;
- *multiplier* - TODO;
- *clock* - the clock object;
- *user_data* - user data that was set or `nil` if no user data was set during connect.

### `on_tick`
Fired when the clock ticks, once every fast clock minute.

Handler: `function (time, clock, user_data)`
- *time* - current time in ticks since midnight, each tick is one fast clock minute;
- *clock* - the clock object;
- *user_data* - user data that was set or `nil` if no user data was set during connect.

### `on_freeze`
Fired when the clock is frozen.

Handler: `function (time, clock, user_data)`
- *time* - current time in ticks since midnight, each tick is one fast clock minute;
- *clock* - the clock object;
- *user_data* - user data that was set or `nil` if no user data was set during connect.
