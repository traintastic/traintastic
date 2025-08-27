# Zones {#zones}

Zones allow you to apply common rules or restrictions to a group of blocks.
For example, you can create a **Staging zone** that mutes trains, or a **Shunting zone** with a speed limit.

- A block can belong to multiple zones.
- A zone can include any blocks, even if they are not physically adjacent.

## Zone list {#zone-list}

Open the zone list from the main menu: *Objects* → *Zones*.
From here you can manage all zones in your world:

- **Add a new zone** — Click the **+** button to create a new zone.
- **Remove a zone** — Select a zone and click the **–** button to delete it.
- **Edit a zone** — Double-click a zone in the list or use the **pencil** button to open its properties.

## Zone properties {#zone-properties}

Each zone has a set of properties that define its behavior.

### Identification
- **Name** — A short description of the zone, e.g. *Shunting Area* or *Staging Area*.
- **ID** — Unique identifier of the zone.
  - Must be unique among all objects.
  - Used for accessing the zone from the [Lua scripting engine](lua.md).
  - Must start with a letter (`a–z`).
  - May only contain lowercase letters (`a–z`), digits (`0–9`), and underscores (`_`).

### Block membership
- **Blocks** — List of blocks included in the zone.
  Use the **+** and **–** buttons to add or remove blocks.
  The **Highlight** button shows all blocks of the zone on opened boards.

### Behavioral settings
- **Mute** — When enabled, all trains in or entering the zone are muted (default: off).
- **No smoke** — When enabled, all smoke generators of trains in or entering the zone are disabled (default: off).
- **Speed limit** — Defines the maximum speed for trains in the zone.
  This does **not** affect manually controlled trains.
  By default, there is no speed limit.
