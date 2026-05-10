# CBUS/VLCB reference

CBUS is a Layout Control Bus developed by Mike Bolton and Gil Fuchs, members of the Model Electronic Railway Group (MERG).
CBUS uses the Controller Area Network (CAN) for communication between the CBUS modules.

VLCB is a CBUS backwards compatible extension developed by MERG members to add additional commands and introduced a stricter priority system for commands.

This appendix does **not** explain the CBUS/VLCB protocol.
Instead, it **how Traintastic implements and uses CBUS/VLCB** and which protocol messages are recognized.
It is intended for advanced users who are already familiar with the basics of the CBUS/VLCB protocol.

## Supported hardware

*TODO: under development*

## Message support

### General
- Node discovery: `QNN`, `PNN` - Partly supported
- Track on/off: `TOF`, `TON`, `RTOF`, `RTON` - Supported
- Emergency stop: `ESTOP`, `RESTP` - Supported
- Command station status: `RSTAT`, `STAT` - Partly supported

### Locomotive control
- Session management: `GLOC`, `KLOC`, `DKEEP`, `PLOC`, `ERR` - Partly supported
- Speed/direction control: `DSPD`, `STMOD` - Supported
- Function control: `DFNON`, `DFNOF` - Supported
- Consisting - Not supported

### Turnouts, signals, and outputs
- Short events: `ASON`, `ASOF` - Supported
- Long events: `ACON`, `ACOF` - Supported

### Feedback sensors
- Short events: `ASON`, `ASOF` - Supported
- Long events: `ACON`, `ACOF` - Supported

### Other
- Sending raw DCC packets: `RDCC3`, `RDCC4`, `RDCC5`, `RDCC6` - Supported

## Debugging and monitoring

Traintastic provides a debug option for CBUS/VLCB that logs all bus traffic.
Messages are shown in **hexadecimal format**, and for many message types a human-readable textual description of the content is also provided.

This is useful for:

- Diagnosing compatibility issues with specific modules.
- Verifying that messages are transmitted and received as expected.

### Sending raw messages

Through [**Lua scripting**](../advanced/scripting-basics.md), it is also possible to:

- Send **raw CBUS/VLCB messages**, see [`send()`](lua/object/cbusinterface.md#send).
- Send **raw DCC track commands** (`RDCCn`), see [`send_dcc()`](lua/object/cbusinterface.md#send_dcc).

!!! warning "Use this with caution!"
    - These messages bypass Traintastic’s normal handling.
    - You need a solid understanding of CBUS/VLCB and DCC to avoid conflicts.
    - Side effects may occur that Traintastic is not aware of or cannot manage.

---

<small>
CBUS® is a registered trademark of Dr Mike Bolton. \
CBUS® protocol documents are a copyright of Mike Bolton and Gil Fuchs.
</small>
