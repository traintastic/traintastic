# XpressNet reference

XpressNet (originally called X-Bus) is a communication bus developed by Lenz Elektronik GmbH.
It is used by Lenz and several other manufacturers, making it possible to mix and match devices from different vendors.
This appendix describes Traintastic's XpressNet implementation details.

## Supported hardware

Traintastic supports a wide range of command stations and interfaces with XpressNet capability.
See the [Supported hardware](supported-hardware.md) page for a complete and up-to-date list.

## Message support

### Power control
- Power on – Supported
- Power off – Supported

### Locomotive control
- Emergency stop all locomotives - Supported
- Emergency stop locomotive - Supported
- Speed & direction control: 14/27/28/128 steps – Supported
- Functions F0–F28: Supported
- Roco MultiMAUS functions F13–F20 – Supported (based on traffic analysis of MultiMAUS communication)
- Consists – **Not supported**, Traintastic manages locomotives individually.

### Turnouts, signals, and outputs
- Accessory control – Supported

### Feedback sensors
- Sensor feedback – Supported

### Fast clock
- OpenDCC custom extension – Not supported

### Programming
- Decoder programming - Planned, not yet supported

## Debugging and monitoring

Traintastic provides a debug option for XpressNet that logs all bus traffic.
Messages are shown in **hexadecimal format**, and for many message types a human-readable textual description of the content is also provided.

This is useful for:

- Diagnosing compatibility issues with specific devices.
- Verifying that messages are transmitted and received as expected.
- Exploring vendor-specific or undocumented extensions of XpressNet.

---

!!! footnote
    For full protocol details, see the official [XpressNet specification in the *23151 Interface LAN und USB* manual (PDF, German)](https://www.lenz-elektronik.de/media/37/8b/2f/1734009949/b_23151.pdf) published by Lenz.
