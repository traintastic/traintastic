# DCC-EX interface configuration

This page describes how to configure a DCC-EX command station in Traintastic.

!!! tip
    The DCC-EX command stations can be added using the **setup wizard**, which guides you through the process step by step.
    See [Quick start: Connect to your command station](../../quickstart/command-station.md) for details.

## Supported connection types

DCC-EX command stations can be connected to Traintastic in two ways:

- **Serial (RS-232/USB)** – Direct cable connection, typically via USB or a USB-to-Serial adapter.
- **Network (TCP/IP)** – Connection over Ethernet or Wi-Fi using the DCC-EX network protocol.

## Connection settings

Depending on the connection type, the following options are available:

### Serial connections
- **Device** – Path to the serial device (e.g., `COM3` on Windows or `/dev/ttyUSB0` on Linux).
- **Baudrate** – Communication speed, must match the DCC-EX configuration.

### Network connections
- **Hostname** – IP address or hostname of the DCC-EX.
- **Port** – TCP port number, must match the DCC-EX configuration. (default: 2560)

## DCC-EX settings

Additional options for fine-tuning behavior:

### Locomotive control

- **Speed steps** – Number of DCC speed steps to use for all locomotives.<br>
  Options: 28 or 128 (default: 128).

### Advanced

- **Startup delay** – Delay (in milliseconds) between establishing the connection and sending the first command.<br>
  Default: 2500 ms (2.5 seconds).

### Debugging

- **Debug log RX/TX** – Log all DCC-EX communication (ascii + description) for debugging purposes.

!!! tip "Need help or having issues?"
    If you encounter problems while configuring DCC-EX, or if your setup behaves differently than expected, check the [community forum](https://discourse.traintastic.org).
    Sharing your configuration and findings helps others and improves Traintastic.
