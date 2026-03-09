# LocoNet interface configuration

This page describes how to configure a LocoNet command station in Traintastic.

!!! tip
    Many command stations can be added using the **setup wizard**, which guides you through the process step by step.
    See [Quick start: Connect to your command station](../../quickstart/command-station.md) for details.

## Supported connection types

LocoNet command stations can be connected to Traintastic in several ways:

- **Serial (RS-232/USB)** – Direct cable connection, often via a USB-to-Serial adapter.
- **Network – Binary protocol** – TCP/IP connection using the LocoNet binary protocol.
- **Network – LBserver protocol** – TCP/IP connection using the LBserver text-based protocol.
- **Network – Z21 protocol** – UDP connection compatible with the Z21 interface.

## Connection settings

Depending on the connection type, the following options are available:

### Serial connections
- **Device** – Path to the serial device (e.g., `COM3` on Windows or `/dev/ttyUSB0` on Linux).
- **Baudrate** – Communication speed, must match the command station’s configuration.
- **Flow control** – Hardware/software flow control setting, usually *None* unless the device requires it.

### Network connections
- **Hostname** – IP address or hostname of the command station.
- **Port** – TCP/UDP port number for the selected protocol.

## LocoNet settings

Additional options for fine-tuning behavior:

### Command station preset

- **Command station** – Select your command station model.<br>
  This preset automatically adjusts related settings (timeouts, slot handling, etc.) to match that station.
  Choosing **Custom** allows you to override and manually configure all options.

### Locomotive control

- **Locomotive slots** – Number of locomotive slots to manage.
- **F9–F28 functions** – Select how extended functions (F9–F28) are controlled.<br>
  Default: Digitrax `OPC_IMM_PACKET`.<br>
  Alternative: Uhlenbrock extended commands.

### Fast clock

- **Fast clock** – Enable or disable fast clock functionality.
- **Fast clock sync enabled** – Synchronize Traintastic’s fast clock with the LocoNet fast clock.
- **Fast clock sync interval** – Interval (seconds) at which synchronization occurs.

### Advanced

- **Echo timeout** – Timeout (ms) while waiting for echo responses.
- **Response timeout** – Timeout (ms) while waiting for command responses.

### Debugging

- **Debug log RX/TX** – Log all LocoNet traffic (hex + description).
- **PCAP capture** – Enable packet capture for debugging.
- **PCAP output file** – File path where captured packets are stored.
- **Listen-only mode** – Passive monitoring mode; no commands are sent.

!!! tip "Need help or having issues?"
    If you encounter problems while configuring LocoNet, or if your setup behaves differently than expected, check the [community forum](https://discourse.traintastic.org).
    Sharing your configuration and findings helps others and improves Traintastic.
