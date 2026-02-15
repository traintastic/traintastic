# DINAMO reference

DINAMO is a track driver system developed by VPEB (Van Perlo Elektronica en Besturingstechniek) in the Netherlands.
It can control **analog and digital (DCC)** trains in mixed operation.

This appendix does **not** explain the DINAMO protocol itself. Instead, it documents **how Traintastic implements and uses DINAMO** and which protocol messages are recognized.
It is intended for advanced users who are already familiar with the basics of the DINAMO protocol.

!!! note
    DINAMO stands for Digital Interface Network for Analog Model Railways.
    The term *analog* refers to the original version of DINAMO, which could only be used to control analog trains.

## Supported hardware

- **RM-C 1/1+/2** - USB interface for communicating with DINAMO
- **TM44** - Track driver module (4 blocks, 4 detectors per block)
- **OC32** - Output controller (32 outputs)

## Message support

Traintastic only support the DINAMO 3.x protocol, the legacy DINAMO 2.x protocol is not supoorted.

Supported protocol features:
- Reading protocol version
- Readgig interface type and version
- Control analog and DCC trains
- Control OC32 module outputs using *SetAspect* commands
- Monitor input events

## Debugging and monitoring

Traintastic provides two debug options for DINAMO:

- **Protocol message logging**
  Logs all messages sent to and received from DINAMO.
  Messages are shown in **hexadecimal format**, and for many message types a **human-readable description** of the decoded content is included.

- **Track driver state logging**
  Logs the internal track driver state.
  For each train, linked blocks and polarity information is logged.

These tools are useful for verifying correct communication and diagnosing unexpected behavior.
