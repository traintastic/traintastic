# LocoNet reference

LocoNet is a network bus developed by Digitrax Inc. in the early 90s for use with their products.
Today, it is used by multiple vendors, and products from different vendors can usually be combined thanks to the standardized bus.
This appendix describes Traintastic’s LocoNet implementation details.

This appendix does **not** explain the LocoNet protocol itself. Instead, it documents **how Traintastic implements and uses LocoNet** and which protocol messages are recognized.
It is intended for advanced users who are already familiar with the basics of the LocoNet protocol.

## Supported hardware

Traintastic supports a wide range of command stations and interfaces with LocoNet capability.
See the [Supported hardware](supported-hardware.md) page for a complete and up-to-date list.

## Implementation philosophy

Traintastic integrates with LocoNet while following its own control model:

- **Direct locomotive control**:
  Traintastic does not use command station *consists*. Instead, it controls each locomotive of a train individually, adjusting speed based on each loco’s speed profile.

- **Minimal slot handling**:
  Slot-related messages are only supported where necessary. Linking, moving, or juggling slots is not (yet) supported.

- **Extended feedback**:
  Beyond the official LocoNet specification, Traintastic also supports additional messages discovered through traffic analysis (e.g., RailCom and Uhlenbrock LISSY feedback).

- **Fast clock support**:
  Traintastic supports the LocoNet fast clock in both roles:
    - **Master:** Traintastic controls the model time and distributes it on the LocoNet bus.
    - **Slave:** Traintastic synchronizes its model time to the fast clock running on LocoNet.

- **Reverse-engineered extensions**:
  Any LocoNet messages not covered by the official *LocoNet Personal Use Edition 1.0 Specification* are supported based on **traffic monitoring and reverse engineering**.

## Message support

### Power control
- Power on: `OPC_GPON` – Supported
- Power off: `OPC_GPOFF` – Supported
- `OPC_IDLE` – Supported

### Locomotive control
- Speed: `OPC_LOCO_SPD` – Supported
- Direction & F0–F4: `OPC_LOCO_DIRF` – Supported
- Functions F5–F8: `OPC_LOCO_SND` – Supported
- Functions F9-F28: `OPC_IMM_PACKET` - Supported
- Functions F9–F28: *Uhlenbrock* – Supported (reverse engineered)
- Slot management: `OPC_MOVE_SLOTS` - Not (yet) supported
- Consists: `OPC_CONSIST_FUNC`, `OPC_LINK_SLOTS`, `OPC_UNLINK_SLOTS` – **Not supported**, Traintastic manages locomotives individually.

### Turnouts, signals, and outputs
- `OPC_SW_REQ` – Supported
- `OPC_SW_REP` – Not supported
- `OPC_SW_STATE` – Not supported
- `OPC_SW_ACK` – Not supported

### Feedback sensors
- Sensor feedback: `OPC_INPUT_REP` – Supported
- RailCom feedback: `OPC_MULTI_SENSE`, `OPC_MULTI_SENSE_LONG` – Supported (reverse engineered)
- Uhlenbrock LISSY (locomotive address, category, direction, speed) – Supported (reverse engineered)

### Slot and system data
- Slot read request: `OPC_RQ_SL_DATA` – Supported
- Slot read: `OPC_SL_RD_DATA` – Supported
- Slot write: `OPC_WR_SL_DATA` – Supported (fast clock)
- Slot status: `OPC_SLOT_STAT1` – Not supported
- Slot move: `OPC_MOVE_SLOTS` - Planned, not yet supported

### Programming
- Decoder programming - Planned, not yet supported
- SV Programming: `OPC_PEER_XFER` - Planned, not yet supported
- LNCV programming: *Uhlenbrock* – Supported (reverse engineered)

## Debugging and monitoring

Traintastic provides a debug option for LocoNet that logs all bus traffic.
Messages are shown in **hexadecimal format**, and for many message types a human-readable textual description of the content is also provided.

This is useful for:

- Diagnosing compatibility issues with specific devices.
- Verifying that messages are transmitted and received as expected.
- Exploring vendor-specific or undocumented extensions of LocoNet.

### Sending raw messages

Through [**Lua scripting**](../advanced/scripting-basics.md), it is also possible to:

- Send **raw LocoNet messages**, see [`send()`](lua/object/loconetinterface.md#send).
- Send **raw DCC track commands** (`OPC_IMM_PACKET`), see [`imm_packet()`](lua/object/loconetinterface.md#imm_packet).

!!! warning "Use this with caution!"
    - These messages bypass Traintastic’s normal handling.
    - You need a solid understanding of LocoNet and DCC to avoid conflicts.
    - Side effects may occur that Traintastic is not aware of or cannot manage.
