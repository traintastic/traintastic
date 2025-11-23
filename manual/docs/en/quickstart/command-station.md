# Quick start: Connect to your command station

After creating your first world, the next step is to connect Traintastic to your **command station / digital system**.
This is done by creating an **interface**, which acts as the bridge between Traintastic and your hardware.

## Step 1: Open the interfaces list

1. Make sure you are in **edit mode** (![pencil](../assets/images/icons/light/edit.png#only-light)![pencil](../assets/images/icons/dark/edit.png#only-dark) button in the top right).
2. In the main menu, go to **Objects → Hardware → Interfaces**.
3. An *Interfaces* dialog will appear. On a new installation, this list is empty.

## Step 2: Start the setup wizard

1. Click the ![plus](../assets/images/icons/dark/circle/add.png) button to add a new interface.
2. In the menu that opens, select **Setup using wizard**.
   - The menu also lists all supported interface types directly, but those entries are intended for experienced users who already know how to configure them manually.

## Step 3: Select your system

In the wizard, choose your **digital system / command station** from the list of hardware.

- The wizard covers the most common systems.
- Not all command stations are listed, but many other models may still work if you configure them manually from the *Interfaces* menu.
  See the [Supported hardware appendix](../appendix/supported-hardware.md) for a complete overview of tested systems.

!!! note
    If your command station is not listed in the wizard, please check the [community forum](https://discourse.traintastic.org).
    Other users may already have experience with your hardware, and your feedback helps us improve Traintastic and expand the wizard with more systems.

Depending on your selection, Traintastic will ask additional questions, such as:

- **How it is connected**: serial, USB, or network (Ethernet or Wi-Fi).
- **Device or port**: for example a COM port (Windows), `/dev/ttyUSB0` (Linux), or an IP address.

!!! tip "Using WiFi"
    Wi-Fi is supported but not recommended for stability reasons.

When you finish the wizard:

- The new interface will appear in the *Interfaces* list.
- A **status icon** will show in the status bar (right side of the window).
    - ![Gray](../assets/images/icons/dark/interface_state.offline.png) — Offline
    - ![Purple](../assets/images/icons/dark/interface_state.initializing.png) — Initializing
    - ![Green](../assets/images/icons/dark/interface_state.online.png) — Online/connected
    - ![Red](../assets/images/icons/dark/interface_state.error.png) — Error occured

If you add multiple interfaces, each has its own status icon.

## Step 4: Connect and test

1. Press the ![connect](../assets/images/icons/light/offline.png#only-light)![connect](../assets/images/icons/dark/offline.png#only-dark) button
   or use the menu: **World → Connection → Connect**.
2. The status icon will change:
     - ![Purple](../assets/images/icons/dark/interface_state.initializing.png) — while initializing (up to a few seconds).
     - ![Green](../assets/images/icons/dark/interface_state.online.png) — if the connection succeeds.
     - ![Red](../assets/images/icons/dark/interface_state.error.png) — if an error occurs.

If an error occurs:

- Open the **server log** (hotkey ++f12++ or **View → Server log**) to see details.
- See [Common interface connection errors](../troubleshooting/interface-connection-errors.md) for steps to resolve typical issues.
- If you’re unsure whether your system is supported, check the [Supported hardware appendix](../appendix/supported-hardware.md).

When the connection succeeds:

Toggle the track power using the
![power off](../assets/images/icons/light/power_off.png#only-light)![power off](../assets/images/icons/dark/power_off.png#only-dark) /
![power on](../assets/images/icons/light/power_on.png#only-light)![power on](../assets/images/icons/dark/power_on.png#only-dark)
button.
If the command station responds, your connection is working!

---

With your command station connected, you’re ready to [add and control locomotives](trains.md).
