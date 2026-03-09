# Scripting Reference – Introduction

Traintastic embeds a [Lua](https://www.lua.org) scripting engine that allows you to extend and automate your model railway world.
This reference describes all scripting features available inside Traintastic: built-in globals, libraries, objects, and custom extensions.

## How to use this reference

This reference is divided into sections:

- **Globals** – predefined variables and functions available everywhere.
- **Libraries** – extended functionality, such as math, string, and logging.
- **Persistent variables** – data that is stored across world save/load.
- **Enums, Sets, Objects** – Traintastic-specific types to interact with the world.
- **Examples** – practical usage samples that combine the above.

!!! tip
    If you are already familiar with Lua, you can skip the language basics and go straight to the [Globals](globals.md).
    If you are new to Lua, read the [Lua language basics](basics.md) first.

## Core Lua vs. Traintastic extensions

Most of Lua’s standard features are available: numbers, strings, tables, control flow, functions, etc.
Traintastic adds its own extensions to let you control the simulation:

- **The `world` global** – the entry point to interact with your world.
- **Enums and sets** – special types to represent model railway concepts.
- **Objects** – live references to trains, vehicles, sensors, switches, and more.

Together, these extensions make it possible to build powerful automations, ranging from simple macros to complex traffic control logic.

## Getting started

To try out scripting:

1. Open the **Lua scripts list** via *Objects → Lua scripts* in the main menu.
2. Create a new script and open it in the script editor.
3. Press **Run all** in the Lua script list to execute it.

## Next steps

- Learn [Lua language basics](basics.md) if you’re new to Lua.
- Explore the [Globals](globals.md) page to see what is available.
- Check out the [Examples](examples.md) for ready-to-use snippets.
