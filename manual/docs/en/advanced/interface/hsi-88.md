# HSI-88 interface configuration

This page describes how to configure an HSI-88 feedback interface in Traintastic.

## Supported connection types

The HSI-88  can only be connected to Traintastic using a direct serial cable connection, or using a USB-to-Serial adapter.

## Connection settings

- **Device** – Path to the serial device (e.g., `COM3` on Windows or `/dev/ttyUSB0` on Linux).

## HSI-88 settings

### Feedback modules

- **Modules left** – Number of 16-port S88 feedback modules connected to the **left** connector.
- **Modules middle** – Number of 16-port S88 feedback modules connected to the **middle** connector.
- **Modules right** – Number of 16-port S88 feedback modules connected to the **right** connector.

### Debugging

- **Debug log RX/TX** – Log all HSI-88 communication (hex) for debugging purposes.

!!! tip "Need help or having issues?"
    If you encounter problems while configuring the HSI-88, or if your setup behaves differently than expected, check the [community forum](https://discourse.traintastic.org).
    Sharing your configuration and findings helps others and improves Traintastic.
