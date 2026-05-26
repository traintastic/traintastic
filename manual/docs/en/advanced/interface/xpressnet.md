# XpressNet interface configuration

This page describes how to configure an XpressNet command station in Traintastic.

!!! tip
    Many command stations can be added using the **setup wizard**, which guides you through the process step by step.
    See [Quick start: Connect to your command station](../../quickstart/command-station.md) for details.

## Supported connection types

XpressNet command stations can be connected to Traintastic in two ways:

- **Serial (RS-232/USB)** – Direct cable connection, often via a USB-to-Serial adapter.
  Some devices provide presets with predefined baud rate and flow control settings.
- **Network** – TCP/IP connection over Ethernet or Wi-Fi.

## Connection settings

Depending on the connection type, the following options are available:

### Serial connections

- **Serial interface type** – Preset for the connected hardware (e.g., RoSoft S88XPressNetLI).<br>
  This preset sets or limits available baudrate and flow control options.
- **Device** – Path to the serial device (e.g., `COM3` on Windows or `/dev/ttyUSB0` on Linux).
- **Baudrate** – Communication speed. Defined by the selected serial interface type.
- **Flow control** – Hardware/software flow control setting, usually *None* unless the device requires it.
- **S88 start address** – (RoSoft S88XPressNetLI only) First address for connected S88 feedback modules.
- **S88 module count** – (RoSoft S88XPressNetLI only) Number of connected S88 feedback modules.

### Network connections

- **Hostname** – IP address or hostname of the command station.
- **Port** – TCP port number.

## XpressNet settings

Additional options for fine-tuning behavior:

### Command station preset

- **Command station** – Select your command station model.<br>
  This preset automatically adjusts related options to match the station.
  Choosing **Custom** allows you to override and configure all options manually.

### Locomotive control

- **Use emergency stop locomotive command** – Enable the dedicated XpressNet emergency stop command for locomotives.
- **Use Roco F13–F20 command** – Enable Roco-specific extended function command for F13–F20.

### Accessory control

- **Use Roco accessory addressing** – Enable Roco-style accessory addressing (alternative to standard XpressNet addressing).

### Debugging

- **Debug log RX/TX** – Log all XpressNet traffic (hex + description).

!!! tip "Need help or having issues?"
    If you encounter problems while configuring XpressNet, or if your setup behaves differently than expected, check the [community forum](https://discourse.traintastic.org).
    Sharing your configuration and findings helps others and improves Traintastic.
