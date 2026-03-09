# Zones

Zones allow you to apply common rules or restrictions to a group of blocks.
For example, you can create a **Staging zone** that mutes trains, or a **Shunting zone** with a speed limit.

- A block can belong to multiple zones.
- A zone can include any blocks, even if they are not physically adjacent.

## Zone list

Open the zone list from the main menu: *Objects* → *Zones*.
From here you can manage all zones in your world:

- **Add a new zone** — Click the ![plus](../assets/images/icons/light/add.png#only-light)![plus](../assets/images/icons/dark/add.png#only-dark) button to create a new zone.
- **Remove a zone** — Select a zone and click the ![bin](../assets/images/icons/light/delete.png#only-light)![bin](../assets/images/icons/dark/delete.png#only-dark) button to delete it.
- **Edit a zone** — Double-click a zone in the list or use the ![pencil](../assets/images/icons/light/edit.png#only-light)![pencil](../assets/images/icons/dark/edit.png#only-dark) button to open its properties.

## Zone properties

Each zone has a set of properties that define its behavior.

### Identification
- **Name** — A short description of the zone, e.g. *Shunting Area* or *Staging Area*.
- **ID** — Unique identifier of the zone.
    - Must be unique among all objects.
    - Used for accessing the zone from the [Lua scripting engine](../advanced/scripting-basics.md).
    - Must start with a letter (`a–z`).
    - May only contain lowercase letters (`a–z`), digits (`0–9`), and underscores (`_`).

### Block membership
- **Blocks** — List of blocks included in the zone.
  Use the ![plus](../assets/images/icons/light/add.png#only-light)![plus](../assets/images/icons/dark/add.png#only-dark) and
  ![minus](../assets/images/icons/light/remove.png#only-light)![minus](../assets/images/icons/dark/remove.png#only-dark) buttons to add or remove blocks.
  The ![highlight](../assets/images/icons/light/highlight_zone.png#only-light)![highlight](../assets/images/icons/dark/highlight_zone.png#only-dark) button shows all blocks of the zone on opened boards.

### Behavioral settings
- **Mute** — When enabled, all trains in or entering the zone are muted (default: off).
- **No smoke** — When enabled, all smoke generators of trains in or entering the zone are disabled (default: off).
- **Speed limit** — Defines the maximum speed for trains in the zone.
  This does **not** affect manually controlled trains.
  By default, there is no speed limit.

!!! note "Mute and no smoke"
    For *mute* to work it is **required** that there is decoder function configured as **Sound** or **Mute**.
    For *no smoke* to work it is **required** that there is decoder function configured as **Smoke**.
