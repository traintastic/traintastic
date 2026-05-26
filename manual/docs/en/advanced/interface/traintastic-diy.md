# Traintastic DIY interface configuration

This page describes how to configure the **Traintastic DIY interface**, which is intended for hobbyists who build their own throttles, input/output devices, or feedback modules.

!!! note
    There is **no setup wizard support** for DIY interfaces.
    Manual configuration is required.

For protocol details and implementation guidance, see the [Traintastic DIY protocol reference](../../appendix/traintastic-diy-protocol.md).

## Supported connection types

DIY devices can connect to Traintastic via:

- **Serial** – Direct RS-232/USB connection.
- **Network** – TCP/IP connection over Ethernet or Wi-Fi.

## Connection settings

### Serial connections
- **Device** – Path to the serial device (e.g., `COM3` on Windows or `/dev/ttyUSB0` on Linux).
- **Baudrate** – Communication speed, must match the DIY device.
- **Flow control** – Hardware/software flow control setting, usually *None*.

### Network connections
- **Hostname** – IP address or hostname of the DIY device.
- **Port** – TCP port number, must match the device configuration.

## DIY settings

- **Startup delay** – Delay (ms) between connecting and sending the first command.
- **Heartbeat timeout** – Maximum time (ms) before connection is considered lost if no heartbeat is received.
- **Debug log RX/TX** – Log all communication traffic (hex + description).
- **Debug log heartbeat** – Log heartbeat messages for monitoring and debugging.

!!! tip "Need help or having issues?"
    If you are building or testing a DIY device and run into problems, check the [community forum](https://discourse.traintastic.org).
    It’s a good place to share your designs and get feedback from other DIY users.
