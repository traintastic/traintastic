# Traintastic DIY protocol

The Traintastic DIY protocol is designed to enable custom hardware development, for example using the Arduino platform, and integrate it with Traintastic.

It is currently supported via:

- **Serial port**: configurable baudrate and flow control, fixed data format 8N1 (8 data bits, no parity, one stop bit)
- **Network connection (TCP)**: configurable port number

## Current limitations

The protocol currently supports:

- Reading inputs  
- Controlling outputs  
- Throttles  

Additional features may be added in the future.

---

## Message format

Each message starts with an opcode byte. The opcode also encodes the payload length in its lower nibble:

- If the lower nibble is `0x0–0xE`: payload length is given directly
- If the lower nibble is `0xF`: the second byte defines the payload length

Every message ends with a checksum byte.  
The checksum is calculated by XOR-ing all previous bytes in the message.

### Examples

No payload:
```

0x50 0x50

```

Fixed payload (4 bytes):
```

0x24 0x11 0x22 0x33 0x44 0x60

```

Extended payload length:
```

0x2F 0x20 ... 0x??

```

---

## Message overview

| Command | Requirement |
|--------|------------|
| Heartbeat | Mandatory |
| Get information | Mandatory |
| Get features | Mandatory |
| Get input state | Mandatory (if input feature enabled) |
| Set input state | Mandatory (if input feature enabled) |
| Get output state | Mandatory (if output feature enabled) |
| Set output state | Mandatory (if output feature enabled) |
| Throttle set function | Mandatory (if throttle feature enabled) |
| Throttle set speed/direction | Mandatory (if throttle feature enabled) |
| Throttle subscribe/unsubscribe | Mandatory (if throttle feature enabled) |

---

## Heartbeat {#heartbeat}

Request:
```

0x00 <checksum>

```

Response:
```

0x00 <checksum>

```

---

## Get information {#get-information}

Request:
```

0xF0 <checksum>

```

Response:
```

0xFF <len> <text...> <checksum>

```

---

## Get features {#get-features}

Request:
```

0xE0 <checksum>

```

Response:
```

0xE4 <FF1> <FF2> <FF3> <FF4> <checksum>

```

FF1 bits:
- bit 0: inputs supported  
- bit 1: outputs supported  
- bit 2: throttle supported  
- bit 3–7: reserved  

---

## Get input state {#get-input-state}

Request:
```

0x12 <AH> <AL> <checksum>

```

Response:
- set input state messages or invalid response

---

## Set input state {#set-input-state}

```

0x13 <AH> <AL> <S> <checksum>

```

S:
- 0x00 unknown
- 0x01 low
- 0x02 high
- 0x03 invalid

---

## Get output state {#get-output-state}

```

0x22 <AH> <AL> <checksum>

```

---

## Set output state {#set-output-state}

```

0x23 <AH> <AL> <S> <checksum>

```

---

## Throttle set speed/direction {#throttle-set-speed-direction}

```

0x37 <TH> <TL> <AH> <AL> <SP> <SM> <FL> <checksum>

```

---

## Throttle set function {#throttle-set-function}

```

0x35 <TH> <TL> <AH> <AL> <FN> <checksum>

```

---

## Subscribe / Unsubscribe {#throttle-sub-unsub}

```

0x34 <TH> <TL> <AH> <AL> <checksum>

