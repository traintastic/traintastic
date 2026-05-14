# Digikeijs DR5033

The **Digikeijs DR5033** is a smart LocoNet booster with basic diagnostic feedback and remote control support.

## Supported diagnostics

- Load
- Temperature *(reported but not functional)*

## Control capabilities

- Power district can be enabled/disabled using an accessory address

## Requirements and notes

- The **temperature value always reports zero**, even though the booster exposes this field
- The **load value remains zero** until the track current reaches approximately **400–500 mA**
- Status feedback is limited compared to newer smart boosters

## Configuration

- **Interface** – The LocoNet interface used to communicate with the booster
- **Address** – Booster LNCV address (factory default: **1**)
- **Polling interval** – Interval (in seconds) at which diagnostic data is requested (default: **5 seconds**)

!!! warning
    Using a **very short polling interval** increases traffic on the LocoNet bus and may negatively affect overall system performance.
    The default value is recommended unless faster updates are really needed.

