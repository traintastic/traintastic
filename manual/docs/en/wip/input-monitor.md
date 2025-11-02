# [[class:input_monitor]]

[[gfx:inputmonitordialog.png]]

The input monitor can be used to monitor a group of inputs on a particular bus e.g. LocoNet or S88. Using the input monitor you can easily test if inputs work as expected or find inputs if their address isn't known.

## Indicator status

|| [[gfx:inputmonitorledoff.png]] || Low ||
|| [[gfx:inputmonitorledoon.png]] || High ||
|| [[gfx:inputmonitorledundefined.png]] || Undefined ||

An input indicator is *undefined* if:

- The input doesn't exist on the bus or
- The input exists on the bus but didn't report its status yet.

[[note:LocoNet inputs report their status after power on, switching track power off and back on should trigger a status report.]]

## Indicator label

White labels indicate that the input is defined in the world, by clicking the input settings dialog can be opend.
Gray labels indicate that the input is not yet defined in the world, if the world is in edit mode, the input can be created by double clicking on it.
