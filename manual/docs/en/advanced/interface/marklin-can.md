# Märklin CAN interface configuration

This page describes how to configure a Märklin CAN command station in Traintastic.

!!! tip
    Märklin CS2/CS3 can be added using the **setup wizard**, which guides you through the process step by step.
    See [Quick start: Connect to your command station](../../quickstart/command-station.md) for details.

## Supported connection types

Märklin CAN command stations can be connected to Traintastic in two ways:

- **Network (TCP/IP)** – Recommended. Connect via Ethernet or Wi-Fi using the Märklin CAN protocol.
- **Serial (RS-232/USB)** – Direct cable connection, often via a USB-to-Serial adapter.

## Connection settings

Depending on the connection type, the following options are available:

### Network connections
- **Hostname** – IP address or hostname of the command station.
- **Port** – TCP port number (default depends on command station model).

### Serial connections
- **Device** – Path to the serial device (e.g., `COM3` on Windows or `/dev/ttyUSB0` on Linux).
- **Baudrate** – Communication speed, determined by the CAN bus interface.
- **Flow control** – Hardware/software flow control setting, usually *None* unless the interface requires it.

!!! note
    Serial connections are used with **CAN bus interfaces** that bridge the Märklin CAN bus to a computer via RS-232 or USB.
    This is different from connecting directly to a Märklin CS2/CS3, which should use the network connection instead.

## Märklin CAN settings

Additional options for fine-tuning behavior:

### General
- **Default switch time** – Default duration (ms) for switch/accessory commands.

### Identification
- **Node UID** – Unique identifier for the CAN node.
- **Node serial number** – Serial number of the CAN node.

Traintastic registers itself as a node on the Märklin CAN bus using the *Node UID* and *Node serial number*, in case of any conflicts they can be adjusted.

### Debugging
- **Debug log RX/TX** – Log all CAN bus traffic (hex + description).
- **Debug status data config** – Log CAN status/configuration messages for diagnostics.
- **Debug config stream** – Log the raw configuration stream.

!!! tip "Need help or having issues?"
    If you encounter problems while configuring Märklin CAN, or if your setup behaves differently than expected, check the [community forum](https://discourse.traintastic.org).
    Sharing your configuration and findings helps others and improves Traintastic.
