# Traintastic Developer Documentation

Welcome to the Traintastic developer documentation, this documentation isn't complete yet, so if you have any questions please ask, then I know what to improve.

This documentation assumes you are familiar with the Traintastic software itself.


## The big picture

Traintastic is a client/server application, all "business logic" is handled by the server, the client is just a visualization of what's going on on the server. When controlling a layout there is only one server, multiple clients can connect to the server to control and/or visualize the layout.


## Core object model

Traintatic is built upon its core object model. Almost all object in Traintastic extend the [`Object`](../server/src/core/object.hpp) class. This `Object` class supports registration of properties, methods and events and has built-in reflection so that is is possible to query and object for its supported properties, methods and events including name, data type information and attributes that provide additional meta data.

The reflection system makes it possible to solve multiple things in a generic way:
- Save/restore a world,
- Network communication and
- Lua scripting.

When new objects are added or existing object are extended, all of the above will work out of the box. There is no need to write any code for save/restore, network communication or scripting. (There are some exceptions ;))


## Network communication

...


## Hardware interfacing

Traintastic has a hardware abstraction layer (HAL), to make it possible to use various types of hardware to control the layout. Due to the HAL most of the software doesn't need to know about which hardware is connected to the software.

For further details see the [hardware](hardware.md) page.
