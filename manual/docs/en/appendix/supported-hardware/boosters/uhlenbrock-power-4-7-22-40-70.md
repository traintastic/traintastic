# Uhlenbrock Power 4/7/22/40/70

The **Uhlenbrock Power** series are smart LocoNet boosters that provide basic diagnostic feedback and remote control support.

## Supported diagnostics

- Load
- Temperature

## Control capabilities

- Power district can be enabled/disabled using an accessory address

## Requirements and notes

- These boosters have **not yet been fully verified** with Traintastic.
  Based on protocol compatibility and documentation, they are expected to work.

## Configuration

- **Interface** – The LocoNet interface used to communicate with the booster
- **Address** – Booster LNCV address (factory default: **1**)
- **Polling interval** – Interval (in seconds) at which diagnostic data is requested (default: **5 seconds**)

!!! warning
    Using a **very short polling interval** increases traffic on the LocoNet bus and may negatively affect overall system performance.
    The default value is recommended unless faster updates are required.

