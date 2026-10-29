# CBUS/VLCB interface configuration

This page describes the configuration of the CBUS/VLCB interface.

## Supported connection types

CBUS/VLCB can be connected to Traintastic in three ways:

- **USB** – Direct cable connection using the CANUSB4 module.
- **Network (TCP/IP)** – Connection via Ethernet or Wi-Fi using the CANEther module.
- **SocketCAN** - Connection via a Linux SocketCAN compatible interface.

## Connection settings

Depending on the connection type, the following options are available:

### USB connection
- **Type** – CANUSB
- **Device** – Path to the serial device (e.g., `COM3` on Windows or `/dev/ttyACM0` on Linux).

### Network connection
- **Type** – CANEther
- **Hostname** – IP address or hostname of the command station/interface.
- **Port** – TCP port number (default value depends on the type of command station/interface).

### SocketCAN connection
- **Type** – SocketCAN (Linux only)
- **Interface** - SocketCAN interface name, e.g. `can0`

## CBUS/VLCB settings

Additional options for fine-tuning behavior:

### Locomotive

- **Keep-alive for locomotives** – Default duration (in seconds) to keep a locomotive session active. Default value: 4 s.

### Accessory

- **DCC accessory switching time** – Default value: 200 ms.

### Protocol

- **CAN ID** - Traintastic's CAN ID on the CBUS/VLCB network. Default value: 122.
- **Node number for short events** – Node number when sending short events. Default value: 0.

### Hub

- **Hub enabled** – Enables a network hub for other programs to access the CBUS device (e.g., JMRI, MMC).
- **Hub for localhost only** – Restricts the hub to the computer running Traintastic.
- **Hub port** – Network port. Default value: 5550.

### Debugging

- **Log all communication** – Log all CBUS/VLCB communication (hex + description) for debugging purposes.

## Nodes

List of nodes discovered on the CBUS/VLCB network.

Traintastic registers itself as a separate node within the CBUS/VLCB network.

## Sessions

List of locomotive session.

!!! tip "Need help or having issues?"
    If you encounter problems while configuring CBUS/VLCB, or if your setup behaves differently than expected, check the [community forum](https://discourse.traintastic.org).
    Sharing your configuration and findings helps others and improves Traintastic.
