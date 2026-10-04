# Events

Many Traintastic objects can notify a Lua script when something happens. This is done using **events**.

There are two kinds of events:

- **Property change events**, using `on_changed()`.
- **Object-specific events**, such as `on_train_entered()` or `on_block_assigned()`.

## Property change events

Every object supports the generic `on_changed()` event.

It can be used to monitor changes to one, multiple, or all properties.

### Single property

The most common use is monitoring a single property.

```lua
signal.on_changed("aspect", function(aspect)
  log.debug("Signal aspect: ", aspect)
end)
```

The callback receives the property's new value.

### Multiple properties

A single callback can also monitor multiple properties.

```lua
turnout.on_changed({"state", "reserved"}, function(value, object, name)
  log.debug(object.name, name, value)
end)
```

The callback is called whenever one of the selected properties changes.

### All properties

To receive notifications for every property of an object, omit the property name.

```lua
block.on_changed(function(value, object, name)
  log.debug(object.name, name, value)
end)
```

## Object-specific events

Some objects provide additional events that describe actions or state changes specific to that object.

Some examples:

- Block: `on_train_entered()`
- Train: `on_block_assigned()`
- Clock: `on_tick()`

Unlike `on_changed()`, these events have callback parameters that are specific to the event.

```lua
block.on_train_entered(function(train, block, direction)
  log.info(train.name .. " entered " .. block.name)
end)

train.on_block_assigned(function(train, block)
  log.info(train.name .. " assigned to " .. block.name)
end)

world.clock.on_tick(function(ticks)
  log.log('current time is ' .. ticks)
end)
```

See the documentation for each object for the events it supports and the callback parameters they receive.

## Reusing callbacks

A callback function can be connected to multiple objects.

The `object` parameter allows the callback to determine which object generated the event.

```lua
function signal_aspect_changed(value, object)
  log.debug(object.name, value)
end

signal1.on_changed("aspect", signal_aspect_changed)
signal2.on_changed("aspect", signal_aspect_changed)
signal3.on_changed("aspect", signal_aspect_changed)
```

This makes it possible to write reusable callback functions instead of creating a separate callback for every object.

## Callback parameters

Lua ignores any callback parameters that are not used. This allows callbacks to receive only the values they need.

For `on_changed()`, the callback receives the following parameters:

| Parameter   | Description                                            |
| ----------- | ------------------------------------------------------ |
| `value`     | The property's new value.                              |
| `object`    | The object whose property changed.                     |
| `name`      | The property name.                                     |
| `user_data` | Optional user data supplied when connecting the event. |

All of the following callback signatures are valid:

```lua
function(value)
```

```lua
function(value, object)
```

```lua
function(value, object, name)
```

```lua
function(value, object, name, user_data)
```

Object-specific events define their own callback parameters. Refer to the documentation of each object for the callback signature of its events.

## User data

An optional `user_data` value can be supplied when connecting an event. Whenever the event occurs, the same value is passed back to the callback.

This is useful when the same callback function is connected multiple times.

```lua
function signal_aspect_changed(value, object, name, user_data)
  log.info(user_data .. ": " .. object.name .. " aspect changed")
end

signal1.on_changed("aspect", signal_aspect_changed, "Main signals")
signal2.on_changed("aspect", signal_aspect_changed, "Main signals")
signal3.on_changed("aspect", signal_aspect_changed, "Shunting signals")
```

## Managing event connections

Registering an event returns a **connection** object.

In most scripts, the return value can simply be ignored. It is only needed when a callback should be temporarily disabled or permanently disconnected.

```lua
local connection = signal.on_changed("aspect", function(aspect)
  log.debug(aspect)
end)
```

The connection object provides the following methods:

| Method         | Description                           |
| -------------- | ------------------------------------- |
| `disconnect()` | Permanently disconnects the callback. |
| `pause()`      | Temporarily disables the callback.    |
| `resume()`     | Re-enables a paused callback.         |

Example:

```lua
local connection = signal.on_changed("aspect", function(aspect)
  log.debug(aspect)
end)

connection.pause()

-- callback is not called

connection.resume()

-- callback is active again

connection.disconnect()

-- callback is permanently removed
```

## Choosing the right event

Use **`on_changed()`** when you want to monitor changes to one or more properties.

```lua
signal.on_changed("aspect", function(aspect)
...
end)
```

Use an **object-specific event** when you are interested in a particular action or occurrence.

```lua
block.on_train_entered(function(train, block, direction)
...
end)
```

Object-specific events are usually easier to read and often provide additional information that is not available through properties alone.
