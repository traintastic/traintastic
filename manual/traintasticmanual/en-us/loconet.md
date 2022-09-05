# LocoNet {#loconet}

LocoNet is a network bus developed by Digitrax inc. in the early 90's for use with their products.
Nowadays LocoNet is used by multiple vendors, products of different vendors can easily be combined due to the standard LocoNet bus.
This appendix describes Traintastic's LocoNet implementation details.

## Supported LocoNet command stations {#loconet-command-stations}

The following LocoNet command stations are verified to work with Traintastic:
- [Digikeijs DR5000](supportedhardware/digikeijs/dr5000.md)
- [Uhlenblock Intellibox 65000](supportedhardware/uhlenbrock/intellibox.md)
- [Uhlenbrock IB-COM 65071](supportedhardware/uhlenbrock/ibcom.md)

LocoNet command stations not listed here will probably work but aren't tested.

## Supported LocoNet interfaces {#loconet-supported-interfaces}

The following LocoNet interfaces are verified to work with Traintastic:
- RoSoft LocoNet interface
- [Uhlenbrock LocoNet interface 63120](supportedhardware/uhlenbrock/loconetinterface63120.md)

LocoNet interfaces not listed here might work but aren't tested.

## Supported LocoNet messages {#loconet-supported-messages}

Traintastic supports most messages defined by Digitrax's *LocoNet Personal Use Edition 1.0 specification* and
some additional messages not covered by the *LocoNet Personal Use Edition 1.0 specification*,
these messages have been retrieved through traffic analysis on the LocoNet network.

Summary of supported LocoNet messages by Traintastic:
- Power control
- Control of locomotive speed and direction
- Control of locomotive functions: F0 … F28
- Reading feedback sensor
- Control of turnouts, signals and outputs
- Fast clock sync ping

Messages defined in the *LocoNet Personal Use Edition 1.0 specification*:

| Opcode             | Supported/Used |
|--------------------|----------------|
| `OPC_IDLE`         | Yes            |
| `OPC_GPON`         | Yes            |
| `OPC_GPOFF`        | Yes            |
| `OPC_BUSY`         | No             |
| `OPC_SW_ACK`       | No             |
| `OPC_SW_STATE`     | No             |
| `OPC_RQ_SL_DATA`   | Yes            |
| `OPC_MOVE_SLOTS`   | No             |
| `OPC_LINK_SLOTS`   | No             |
| `OPC_UNLINK_SLOTS` | No             |
| `OPC_CONSIST_FUNC` | No             |
| `OPC_SLOT_STAT1`   | No             |
| `OPC_LONG_ACK`     | Partly         |
| `OPC_INPUT_REP`    | Yes            |
| `OPC_SW_REP`       | No             |
| `OPC_SW_REQ`       | Yes            |
| `OPC_LOCO_SND`     | Yes            |
| `OPC_LOCO_DIRF`    | Yes            |
| `OPC_LOCO_SPD`     | Yes            |
| `OPC_WR_SL_DATA`   | No             |
| `OPC_SL_RD_DATA`   | Yes            |
| `OPC_PEER_XFER`    | No             |
| `OPC_IMM_PACKET`   | No             |

Messages retrieved through traffic analysis:
- Control of locomotive functions: F9 … F28
