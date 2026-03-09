# Lua Language Basics

This page introduces the basic syntax and concepts of the Lua programming language.
If you are already familiar with Lua, you can skip this page and continue with the [Globals](globals.md).

Lua is a lightweight and easy-to-learn scripting language. It is designed to be simple, flexible, and embeddable.
Traintastic uses Lua to allow you to write scripts that interact with your model railway world.

## Variables

Variables are used to store values.

```lua
name = "Traintastic"
speed = 80
enabled = true
```

- Strings are written in quotes (`"text"`).
- Numbers can be integers or decimals.
- Booleans are `true` or `false`.

## Comments

Comments are ignored by Lua but help you explain your code.

```lua
-- This is a single-line comment

--[[
This is a
multi-line comment
]]
```

## Control structures

Lua uses familiar programming constructs.

### If / else
```lua
if speed > 60 then
  log.info("Fast!")
else
  log.info("Slow.")
end
```

### Loops
```lua
for i = 1, 5 do
  log.debug("Step", i)
end
```

```lua
while enabled do
  log.debug("Running...")
  break
end
```

## Functions

Functions group code into reusable blocks.

```lua
function greet(name)
  log.info("Hello " .. name)
end

greet("Traintastic")
```

## Tables

Tables are Lua’s only data structure. They work as lists, dictionaries, or objects.

```lua
car = { type = "freight", cargo = "coal" }
log.debug(car.type)        -- "freight"

numbers = { 10, 20, 30 }
log.debug(numbers[1])      -- 10
```

## Putting it together

With just these basics—variables, control flow, functions, and tables—you can already create useful scripts in Traintastic.

!!! tip
    Don’t worry about learning everything at once. Start simple and build up as you go.

## Next steps

- See what Traintastic adds on top of Lua in [Globals](globals.md).
- Explore [Persistent variables](pv.md) to keep data between sessions.
- Browse the [Examples](examples.md) to learn by doing.
