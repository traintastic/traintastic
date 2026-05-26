# Traintastic DIY protocol

The Traintastic DIY protocol is designed to make it possible to develop custom hardware, e.g. by using the Arduino platform and use it with Traintastic.

The Traintastic DIY protocol is currently supported via:

- Serial port: baudrate and flow control can be chosen, data format is fixed at 8N1 (8 data bits, no parity, one stop bit)
- Network connection (TCP): port number can be chosen.

It is currently limited to:

- Reading inputs
- Controlling outputs
- Throttles

Other features might be added in the future.

## Message format

Each Traintastic DIY protocol message starts with an opcode byte, besides the message type it also contains the data payload length in the lowest nibble.
If the lowest nibble is `0xF` the the second byte of the message determines the payload length.
The message always ends with a checksum byte, the checksum is the result of XOR-ing of all message bytes.

Examples:
```
0x50 0x50
```
The lowest nibble of the first byte is `0` indicating a zero byte payload.
The checksum is identical to the opcode, there is no data to XOR with.

```
0x24 0x11 0x22 0x33 0x44 0x60
```
The lowest nibble of the first byte is `4` indicating a 4 byte payload.
The checksum is `0x24` XOR `0x11` XOR `0x22` XOR `0x33` XOR `0x44` = `0x60`.

```
0x2F 0x20 ... 0x??
```
The lowest nibble of the first byte is `F` indicating that the second byte must be used as payload length, 32 byte.
The checksum is `0x2F` XOR `0x20` XOR *all payload bytes*.


## Messages

Messages are send by Traintastic to the DIY device, for most messages the DIY device sends a response message.
Some messages are sent unsolicited by the DIY device to Traintastic if changes are detected by the DIY device.

| Command                                                       |                                           |
|---------------------------------------------------------------|-------------------------------------------|
| [Heartbeat](#heartbeat)                                       | Mandatory                                 |
| [Get information](#get-information)                           | Mandatory                                 |
| [Get features](#get-features)                                 | Mandatory                                 |
| [Get input state](#get-input-state)                           | Mandatory if input feature flag is set    |
| [Set input state](#set-input-state)                           | Mandatory if input feature flag is set    |
| [Get output state](#get-output-state)                         | Mandatory if output feature flag is set   |
| [Set output state](#set-output-state)                         | Mandatory if output feature flag is set   |
| [Throttle set function](#throttle-set-function)               | Mandatory if throttle feature flag is set |
| [Throttle set speed/direction](#throttle-set-speed-direction) | Mandatory if throttle feature flag is set |
| [Throttle subscribe/unsubscribe](#throttle-sub-unsub)         | Mandatory if throttle feature flag is set |


### Heartbeat {#heartbeat}

The heartbeat message is sent by Traintastic to check if the DIY device is (still) present, the DIY device responds with a heartbeat message.
The heartbeat rate can be configured in Traintastic, by default the heartbeat message is one second after the last message is received from the DIY device.

#### Request message
```
0x00 <checksum>
```

#### Response message
```
0x00 <checksum>
```


### Get information {#get-information}

The *get information* message is the first message sent after connecting.
The DIY device responds with an *information* message containing a description of the connected DIY device.
This is pure informational and displayed in the message console.

#### Request message
```
0xF0 <checksum>
```

#### Response message
```
0xFF <len> <text...> <checksum>
```

### Get features {#get-features}

The *get features* message is the second message sent by Traintastic after connecting.
The DIY device responds with a *features* message containing flags which indicate what is supported by the DIY device.

#### Request message
```
0xE0 <checksum>
```

#### Response message
```
0xE4 <FF1> <FF2> <FF3> <FF4> <checksum>
```

- `<FF1>` feature flags 1:
    - bit 0: input feature flag: set if the DIY device has inputs
    - bit 1: output feature flag: set if the DIY device has outputs
    - bit 2: throttle feature flag: set if the DIY device is a throttle
    - bit 3...7: reserved, must be `0`
- `<FF2>` feature flags 2, reserved must be `0x00`
- `<FF3>` feature flags 3, reserved must be `0x00`
- `<FF4>` feature flags 4, reserved must be `0x00`


### Get input state {#get-input-state}

Sent by Traintastic to retrieve the current input state.
Address zero has a special meaning, it is used as broadcast address to retrieve the current state of all inputs.

#### Request message
```
0x12 <AH> <AL> <checksum>
```

- `<AH>` high byte of 16bit input address
- `<AL>` low byte of 16bit input address

#### Response
If the address is non zero the DIY device responds with a *[set input state](#set-input-state)* message containing the current state of the input address.

If the address is zero the DIY device responds with multiple *[set input state](#set-input-state)* messages, one for each know input address or
send a single *[set input state](#set-input-state)* message with address zero and state *invalid* to inform Traintastic that the address zero request is not supported.


### Set input state {#set-input-state}

Sent by the DIY device as response to the *[get input state](#get-input-state)* message and must be sent by the DIY device whenever an input state changes.

#### Message
```
0x13 <AH> <AL> <S> <checksum>
```

- `<AH>` high byte of 16bit input address
- `<AL>` low byte of 16bit input address
- `<S>` input state:
    - `0x00` if input state is unknown
    - `0x01` if input state is low/false
    - `0x02` if input state is high/true
    - `0x03` if input is invalid (only as response to a *[get input state](#get-input-state)* message)
    - `0x04`...`0xFF` are reserved, do not use

#### Examples
```
0x13 0x00 0x12 0x02 0x03
```
Input 18 state changed to high/true

```
0x13 0x02 0xA2 0x01 0xB2
```
Input 674 state changed to low/false


### Get output state {#get-output-state}

Sent by Traintastic to retrieve the current output state.
Address zero has a special meaning, it is used as broadcast address to retrieve the current state of all outputs.

#### Request message
```
0x22 <AH> <AL> <checksum>
```

- `<AH>` high byte of 16bit output address
- `<AL>` low byte of 16bit output address

#### Response message
If the address is non zero the DIY device responds with a *[set output state](#set-output-state)* message containing the current state of the output address.

If the address is zero the DIY device responds with multiple *[set inpoutputut state](#set-output-state)* messages, one for each know output address or
send a single *[set output state](#set-output-state)* message with address zero and state *invalid* to inform Traintastic that the address zero request is not supported.


### Set output state {#set-output-state}

Sent by Traintastic to change the state of an output, the DIY device responds with a *get output state* message containing the new output state,
if for some reason the output state cannot be the current state must be send.
Sent by the DIY device as response to the *[get output state](#get-output-state)* message and must be sent by the DIY device whenever an output state changes.

#### Message
```
0x23 <AH> <AL> <S> <checksum>
```

- `<AH>` high byte of 16bit output address
- `<AL>` low byte of 16bit output address
- `<S>` output state:
    - `0x00` if output state is unknown
    - `0x01` if output state is low/false
    - `0x02` if output state is high/true
    - `0x03` if output is invalid (only as response to a *[get output state](#get-output-state)* message)
    - `0x04`...`0xFF` are reserved, do not use


### Throttle set speed/direction {#throttle-set-speed-direction}

Set locomotive decoder speed and/or direction.

Once a *Throttle set speed/direction* message is sent, the *throttle id* will automatically be subscribed for speed, direction and function changes of the locomotive decoder specified by the *decoder address*.
To stop receiving these changes a *[throttle unsubscribe](#throttle-sub-unsub)* message has to be send to Traintastic by the DIY device.

#### Message
```
0x37 <TH> <TL> <AH> <AL> <SP> <SM> <FL> <checksum>
```

- `<TH>` high byte of 16bit throttle id
- `<TL>` low byte of 16bit throttle id
- `<AH>`:
    - bit 0...5: highest 6bit of 14bit decoder address
    - bit 6 is reserved and must be `0`
    - bit 7 can be set to force a *DCC long address*
- `<AL>` lowest 8bit of 14bit decoder address
- `<SP>` speed (step), `0`...`<SM>`
- `<SM>` maximum speed (step), set to `0` for emergency stop
- `<FL>` flags:
    - bit 0: direction `1`=forward, `0`=reverse
    - bit 1...5: reserved, must be `0`
    - bit 6: set to set direction
    - bit 7: set to set speed

*Throttle id* can be used to distinguish different throttles within the DIY device if it represents multiple throttles. If the DIY device is a single throttle use `0x00` `0x00` as *throttle id*.

Internally Traintastic uses a value between `0` and `1` for the speed where `0` is stop and `1` is full speed. To determine a value between `0` and `1` Traintastic calculates `<SP> / <SM>`. Set `<SP>` and `<SM>` both to zero for an emergency stop.

Using the *flags* bit 6 and 7 it is possible to set speed and direction, just the speed, just the direction or nothing. Setting nothing still subscribes the *throttle id* for speed, direction and function changes.

#### Examples
```
0x37 0x00 0x01 0x00 0x03 0x07 0x0E 0xC1 0xFD
```
Set speed to 50% (= 7 / 14) in forward direction for decoder with address 3.

```
0x37 0x00 0x01 0x00 0x03 0x00 0x00 0x80 0xB5
```
Emergency stop decoder with address 3, don't change direction.


### Throttle set function {#throttle-set-function}

Enable/disable locomotive decoder function.

Once a *Throttle set function* message is sent, the *throttle id* will automatically be subscribed for speed, direction and function changes of the locomotive decoder specified by the *decoder address*.
To stop receiving these changes a *[throttle unsubscribe](#throttle-sub-unsub)* message has to be send to Traintastic by the DIY device.

#### Message
```
0x35 <TH> <TL> <AH> <AL> <FN> <checksum>
```

- `<TH>` high byte of 16bit throttle id
- `<TL>` low byte of 16bit throttle id
- `<AH>`:
    - bit 0...5: highest 6bit of 14bit decoder address
    - bit 6: reserved and must be `0`
    - bit 7: set to force a *DCC long address*
- `<AL>` lowest 8bit of 14bit decoder address
- `<FN>`:
    - bit 0...6: function number
    - bit 7: function value

#### Examples
```
0x35 0x00 0x01 0x00 0x03 0x80 0xB7
```
Enable F0 for decoder with address 3.

```
0x35 0x00 0x02 0x80 0x05 0x01 0xB3
```
Disable F1 for decoder with long address 5.


### Throttle subscribe/unsubscribe {#throttle-sub-unsub}

Subscribe/unsubscribe for change events. Traintastic will send a *[throttle set speed/direction](#throttle-set-speed-direction)* message whenever speed or direction changes and a *[throttle set function](#throttle-set-function)* message for every function that changes state.

!!! note
    Subscribe is supported since v0.3, older version only support unsubscribe.

#### Message
```
0x34 <TH> <TL> <AH> <AL> <checksum>
```

- `<TH>` high byte of 16bit throttle id
- `<TL>` low byte of 16bit throttle id
- `<AH>`:
    - bit 0...5: highest 6bit of 14bit decoder address
    - bit 6: action: `0` = Unsubscribe, `1` = Subscribe
    - bit 7: set to force a *DCC long address*
- `<AL>` lowest 8bit of 14bit decoder address

When *subscribing* Traintastic will reply with a *[throttle set speed/direction](#throttle-set-speed-direction)* and a *[throttle set function](#throttle-set-function)* message for every function that is known for the address.

When *unsubscribing* Traintastic will reply with the same message to confirm the unsubscribe.
