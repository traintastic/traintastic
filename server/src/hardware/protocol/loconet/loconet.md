# LocoNet


## DR5000 - loconet 0 using Multimaus

2020-07-16 22:20:38.953136 [debug]    cs_1.loconet: rx: A3 [A3 1F 01 42] -> F9
2020-07-16 22:20:46.782380 [debug]    cs_1.loconet: rx: A3 [A3 1F 00 43]
2020-07-16 22:21:13.039958 [debug]    cs_1.loconet: rx: A3 [A3 1F 02 41] -> F10
2020-07-16 22:21:15.329510 [debug]    cs_1.loconet: rx: A3 [A3 1F 00 43]
2020-07-16 22:21:20.004596 [debug]    cs_1.loconet: rx: A3 [A3 1F 04 47] -> F11
2020-07-16 22:21:20.644998 [debug]    cs_1.loconet: rx: A3 [A3 1F 00 43]
2020-07-16 22:21:21.413525 [debug]    cs_1.loconet: rx: A3 [A3 1F 08 4B] -> F12
2020-07-16 22:21:21.893828 [debug]    cs_1.loconet: rx: A3 [A3 1F 00 43]


F13 ON:
2020-07-16 22:49:50.937588 [debug]    cs_1.loconet: rx: D4 [D4 20 1F 08 01 1D]
2020-07-16 22:49:50.937756 [debug]    cs_1.loconet: rx: D4 [D4 20 1F 05 00 11] // why??
F13 OFF:
2020-07-16 22:49:53.723441 [debug]    cs_1.loconet: rx: D4 [D4 20 1F 08 00 1C]
2020-07-16 22:49:53.723604 [debug]    cs_1.loconet: rx: D4 [D4 20 1F 05 00 11]

F14 ON:
2020-07-16 22:50:54.355985 [debug]    cs_1.loconet: rx: D4 [D4 20 1F 08 02 1E]
2020-07-16 22:50:54.356150 [debug]    cs_1.loconet: rx: D4 [D4 20 1F 05 00 11]
F14 OFF:
2020-07-16 22:50:56.821657 [debug]    cs_1.loconet: rx: D4 [D4 20 1F 08 00 1C]
2020-07-16 22:50:56.821816 [debug]    cs_1.loconet: rx: D4 [D4 20 1F 05 00 11]

F15 ON:
2020-07-16 22:51:18.564022 [debug]    cs_1.loconet: rx: D4 [D4 20 1F 08 04 18]
2020-07-16 22:51:18.564195 [debug]    cs_1.loconet: rx: D4 [D4 20 1F 05 00 11]
F16 OFF:
2020-07-16 22:51:20.933660 [debug]    cs_1.loconet: rx: D4 [D4 20 1F 08 00 1C]
2020-07-16 22:51:20.949613 [debug]    cs_1.loconet: rx: D4 [D4 20 1F 05 00 11]

F20 ON:
2020-07-17 11:00:28.212622 [debug]    cs_1.loconet: rx: D4 [D4 20 1F 09 01 1C]






## DR5088RC - RailCom

OPC_MULTI_SENSE standard (dir=off):
enter = OPC_MULTI_SENSE enter sensorAddress=15 transponderAddress=3311 20 0F 19 6F 76
leave = OPC_MULTI_SENSE leave sensorAddress=15 transponderAddress=3311 00 0F 19 6F 56

OPC_MULTI_SENSE standard (dir=block +2048):
enter dir A = OPC_MULTI_SENSE enter sensorAddress=15   transponderAddress=3311 20 0F 19 6F 76
enter dir B = OPC_MULTI_SENSE enter sensorAddress=2063 transponderAddress=3311 30 0F 19 6F 66
leave dir A = OPC_MULTI_SENSE leave sensorAddress=15   transponderAddress=3311 00 0F 19 6F 56
leave dir B = OPC_MULTI_SENSE leave sensorAddress=2063 transponderAddress=3311 10 0F 19 6F 46

OPC_MULTI_SENSE standard (dir=address +4096)
enter dir A = OPC_MULTI_SENSE enter sensorAddress=15 transponderAddress=3311 20 0F 19 6F 76
enter dir B = OPC_MULTI_SENSE enter sensorAddress=15 transponderAddress=7407 20 0F 39 6F 56
leave dir A = OPC_MULTI_SENSE leave sensorAddress=15 transponderAddress=3311 00 0F 19 6F 56
leave dir B = OPC_MULTI_SENSE leave sensorAddress=15 transponderAddress=7407 00 0F 39 6F 76

OPC_MULTI_SENSE long:
enter dir A = E0 09 20 0F 19 6F 40 00 0F
enter dir B = E0 09 20 0F 19 6F 00 00 4F
leave dir A = E0 09 00 0F 19 6F 40 00 2F
leave dir B = E0 09 00 0F 19 6F 00 00 6F


## Uhlenbrock 68610 - LISSY receiver

### ???
```2021-06-19 22:18:51.856532 [debug]    cs_1.loconet: rx: OPC_PEER_XFER [E5 0F 00 49 4B 24 00 00 00 00 00 00 00 78 4B]```
```2021-06-19 22:18:59.502691 [debug]    cs_1.loconet: rx: OPC_IMM_PACKET [ED 0F 01 05 00 21 33 7F 7F 00 00 7F 7F 00 0B]```

### LNCV programming on??
```2021-06-19 22:19:32.454136 [debug]    cs_1.loconet: rx: OPC_IMM_PACKET [ED 0F 01 05 00 21 41 4D 1A 00 00 01 00 00 2F]```
```2021-06-19 22:19:32.470128 [debug]    cs_1.loconet: rx: OPC_PEER_XFER [E5 0F 05 49 4B 1F 01 4D 1A 00 00 01 00 00 5A]```

```
2022-03-10 15:23:12.802948 loconet_1 D2002: RX: OPC_IMM_PACKET [ED 0F 01 05 00 21 41 74 18 00 00 01 00 00 14]
2022-03-10 15:23:12.818892 loconet_1 D2002: RX: OPC_PEER_XFER [E5 0F 05 49 4B 1F 01 74 18 00 00 01 00 00 61]

module = 63880
address = 1
```

```
2022-03-10 15:26:44.290989 loconet_1 D2002: RX: OPC_IMM_PACKET [ED 0F 01 05 00 21 41 4D 1A 00 00 01 00 00 2F]
2022-03-10 15:26:44.307022 loconet_1 D2002: RX: OPC_PEER_XFER [E5 0F 05 49 4B 1F 01 4D 1A 00 00 01 00 00 5A]

module = 68610
address = 1
```

```
2022-03-10 15:28:20.034279 loconet_1 D2002: RX: OPC_IMM_PACKET [ED 0F 01 05 00 21 41 4D 1A 00 00 02 00 00 2C]

module = 68610
address = 2

no response, so it doesn't exist I guess
```

```
2022-03-10 15:31:17.482750 loconet_1 D2002: RX: OPC_IMM_PACKET [ED 0F 01 05 00 21 51 4D 1A 00 00 7F 00 00 41]

module = 68610
address = 255

no response, so it doesn't exist
```

```
2022-03-10 15:32:44.592787 loconet_1 D2002: RX: OPC_IMM_PACKET [ED 0F 01 05 00 21 51 4D 1A 00 00 7F 03 00 42]

module = 68610
address = 1023

no response, so it doesn't exist
```

```
2022-03-10 15:57:55.128820 loconet_1 D2002: RX: OPC_IMM_PACKET [ED 0F 01 05 00 21 51 4D 1A 00 00 7F 0F 00 4E]

module = 68610
address = 4095

no response, so it doesn't exist
```

```
2022-03-10 15:59:27.716753 loconet_1 D2002: RX: OPC_IMM_PACKET [ED 0F 01 05 00 21 51 4D 1A 00 00 7F 1F 00 5E]

module = 68610
address = 8191

no response, so it doesn't exist
```

```
2022-03-10 16:02:19.704231 loconet_1 D2002: RX: OPC_IMM_PACKET [ED 0F 01 05 00 21 71 4D 1A 00 00 7F 7F 00 1E]
2022-03-10 16:02:19.704362 loconet_1 D2002: RX: OPC_PEER_XFER [E5 0F 05 49 4B 1F 01 4D 1A 00 00 01 00 00 5A]

module = 68610
address = 65535 = 0xFFFF

response! broadcast?
```

summary:
```
Module: 68610
    0 => [ED 0F 01 05 00 21 41 4D 1A 00 00 00 00 00 2E]
    1 => [ED 0F 01 05 00 21 41 4D 1A 00 00 01 00 00 2F]
    2 => [ED 0F 01 05 00 21 41 4D 1A 00 00 02 00 00 2C]
    3 => [ED 0F 01 05 00 21 41 4D 1A 00 00 03 00 00 2D]
  127 => [ED 0F 01 05 00 21 41 4D 1A 00 00 7F 00 00 51]
  128 => [ED 0F 01 05 00 21 51 4D 1A 00 00 00 00 00 3E]
  129 => [ED 0F 01 05 00 21 51 4D 1A 00 00 01 00 00 3F]
  511 => [ED 0F 01 05 00 21 51 4D 1A 00 00 7F 01 00 40]
65535 => [ED 0F 01 05 00 21 71 4D 1A 00 00 7F 7F 00 1E] => broadcast
Module: 63880
65535 => [ED 0F 01 05 00 21 71 74 18 00 00 7F 7F 00 25]
Module: 0
    0 => [ED 0F 01 05 00 21 40 00 00 00 00 00 00 00 78]
65535 => [ED 0F 01 05 00 21 70 00 00 00 00 7F 7F 00 48]
                            ^^ ^^ ^^       ^^ ^^
                            B8 ML MH       AL AH

B8 is used for 7th bit's

AL = address bit 0..6
AH = address bit 8..14
B8: 0x40 |
- 0x10 = address bit 7
- 0x20 = address bit 15

ML MH = module address
4D 1A => 68610
74 18 => 63880
^^ ^^
Lo Hi

0x80 | 0x4d | (0x1a << 8) => 6861

0x80 | 0x74 | (0x18 << 8) => 6388

module number = div 10

response:
Module: 68610
     1 => [E5 0F 05 49 4B 1F 01 4D 1A 00 00 01 00 00 5A]
                             ^^ ^^ ^^       ^^ ^^
                             B8 ML MH       AL AH
```

### LNCV 0 -> 5
```2021-06-19 21:17:57.749146 [debug]    cs_1.loconet: rx: OPC_IMM_PACKET [ED 0F 01 05 00 20 01 4D 1A 00 00 05 00 00 6A]```
```2021-06-19 21:17:57.764779 [debug]    cs_1.loconet: rx: OPC_LONG_ACK [B4 6D 7F 59]```

### LNCV 0 -> 1
```2021-06-19 21:18:34.123620 [debug]    cs_1.loconet: rx: OPC_IMM_PACKET [ED 0F 01 05 00 20 01 4D 1A 00 00 01 00 00 6E]```

### LNCV 1 -> 2
```2021-06-19 21:21:37.568839 [debug]    cs_1.loconet: rx: OPC_IMM_PACKET [ED 0F 01 05 00 20 01 4D 1A 01 00 02 00 00 6C]```
```2021-06-19 21:21:37.584419 [debug]    cs_1.loconet: rx: OPC_LONG_ACK [B4 6D 7F 59]```

### LNCV 2 -> 1
```2021-06-19 21:23:07.769809 [debug]    cs_1.loconet: rx: OPC_IMM_PACKET [ED 0F 01 05 00 20 01 4D 1A 02 00 01 00 00 6C]```
```2021-06-19 21:23:07.785617 [debug]    cs_1.loconet: rx: OPC_LONG_ACK [B4 6D 7F 59]```

### LNCV 15 -> 1
```2021-06-19 21:25:23.255584 [debug]    cs_1.loconet: rx: OPC_IMM_PACKET [ED 0F 01 05 00 20 01 4D 1A 0F 00 01 00 00 61]```
```2021-06-19 21:25:23.271245 [debug]    cs_1.loconet: rx: OPC_LONG_ACK [B4 6D 7F 59]```

### LNCV programming off??
```2021-06-19 21:40:20.590313 [debug]    cs_1.loconet: rx: OPC_PEER_XFER [E5 0F 01 05 00 21 03 7F 7F 00 00 01 00 40 72]```

```
2022-03-11 19:14:13.177378 loconet_1 D2002: RX: OPC_IMM_PACKET [ED 0F 01 05 00 21 11 74 18 00 00 7F 00 40 7A]

module = 63880
address = 255
```

### LNCV write
```
LNCV  0 -> 5 : OPC_IMM_PACKET [ED 0F 01 05 00 20 01 4D 1A 00 00 05 00 00 6A]
LNCV  0 -> 1 : OPC_IMM_PACKET [ED 0F 01 05 00 20 01 4D 1A 00 00 01 00 00 6E]
LNCV  1 -> 2 : OPC_IMM_PACKET [ED 0F 01 05 00 20 01 4D 1A 01 00 02 00 00 6C]
LNCV  2 -> 1 : OPC_IMM_PACKET [ED 0F 01 05 00 20 01 4D 1A 02 00 01 00 00 6C]
LNCV 15 -> 1 : OPC_IMM_PACKET [ED 0F 01 05 00 20 01 4D 1A 0F 00 01 00 00 61]
                               ^^ ^^ ?? ?? ?? ?? ?? ^^^^^ ^^^^^ ^^^^^
                              OPC  |               Module LNCV  Value
                              IMM len               Lo Hi Lo Hi Lo Hi
```

```
2022-03-11 19:06:19.252118 loconet_1 D2002: RX: OPC_IMM_PACKET [ED 0F 01 05 00 20 11 74 18 00 00 7F 00 00 3B]
```

Response = long ack
```
2022-10-08 17:21:36.955220 loconet_1 D2002: RX: OPC_LONG_ACK [B4 6D 7F 59]
```


### LNCV read
```
2022-03-11 19:02:14.989146 loconet_1 D2002: RX: OPC_IMM_PACKET [ED 0F 01 05 00 21 01 74 18 01 00 0F 00 00 5B]
2022-03-11 19:02:15.005091 loconet_1 D2002: RX: OPC_PEER_XFER [E5 0F 05 49 4B 1F 01 74 18 01 00 02 00 00 63]

module = 63880
address = 15
lncv = 1
value = 2
```

```
2022-03-11 20:53:12.608484 loconet_1 D2002: RX: OPC_IMM_PACKET [ED 0F 01 05 00 21 01 4D 1A 01 00 01 00 00 6E]
2022-03-11 20:53:12.624477 loconet_1 D2002: RX: OPC_PEER_XFER [E5 0F 05 49 4B 1F 01 4D 1A 01 00 02 00 00 58]

module = 68610
address = 1
lncv = 1
value = 2
```


Start/Read LNCV 1 with IB-com util
```
2022-10-08 16:09:19.373818 loconet_1 D2002: RX: OPC_IMM_PACKET LNCV read: module=6388 address=1 lncv=0 [ED 0F 01 05 00 21 41 74 18 00 00 01 00 00 14]
2022-10-08 16:09:19.394023 loconet_1 D2002: RX: OPC_PEER_XFER LNCV read response: module=6388 lncv=0 value=1 [E5 0F 05 49 4B 1F 01 74 18 00 00 01 00 00 61]
2022-10-08 16:09:19.409595 loconet_1 D2002: RX: OPC_PEER_XFER LNCV read response: module=6388 lncv=0 value=1 [E5 0F 05 49 4B 1F 01 74 18 00 00 01 00 00 61]
2022-10-08 16:09:24.932170 loconet_1 D2002: RX: OPC_IMM_PACKET [ED 0F 01 05 00 21 01 74 18 00 00 01 00 00 54]
2022-10-08 16:09:24.959117 loconet_1 D2002: RX: OPC_PEER_XFER LNCV read response: module=6388 lncv=0 value=1 [E5 0F 05 49 4B 1F 01 74 18 00 00 01 00 00 61]
2022-10-08 16:09:24.973951 loconet_1 D2002: RX: OPC_PEER_XFER LNCV read response: module=6388 lncv=0 value=1 [E5 0F 05 49 4B 1F 01 74 18 00 00 01 00 00 61]
```

**Start = read of lncv 0 with one extra bit set**


## Uhlenbrock IB-COM

### Read SO

Read SO 1:
```
2022-03-07 23:42:12.508821 loconet_1 D2001: TX: OPC_IMM_PACKET [ED 0F 01 49 42 02 00 01 00 00 00 00 00 00 14]
2022-03-07 23:42:12.522811 loconet_1 D2002: RX: OPC_IMM_PACKET [ED 0F 01 49 42 02 00 01 00 00 00 00 00 00 14]
2022-03-07 23:42:12.531876 loconet_1 D2002: RX: OPC_PEER_XFER [E5 0F 00 49 4B 00 00 01 00 00 00 00 00 00 16]
```

Read SO 2:
```
2022-03-07 23:42:12.540684 loconet_1 D2001: TX: OPC_IMM_PACKET [ED 0F 01 49 42 02 00 02 00 00 00 00 00 00 17]
2022-03-07 23:42:12.554615 loconet_1 D2002: RX: OPC_IMM_PACKET [ED 0F 01 49 42 02 00 02 00 00 00 00 00 00 17]
2022-03-07 23:42:12.563683 loconet_1 D2002: RX: OPC_PEER_XFER [E5 0F 00 49 4B 00 00 02 00 04 00 00 00 00 11]
```

Read SO 255:
```
2022-03-07 23:42:12.564207 loconet_1 D2001: TX: OPC_IMM_PACKET [ED 0F 01 49 42 02 01 7F 00 00 00 00 00 00 6B]
2022-03-07 23:42:12.579173 loconet_1 D2002: RX: OPC_IMM_PACKET [ED 0F 01 49 42 02 01 7F 00 00 00 00 00 00 6B]
2022-03-07 23:42:12.588373 loconet_1 D2002: RX: OPC_PEER_XFER [E5 0F 00 49 4B 00 05 7F 00 7F 00 00 00 00 12]
```

### Serial number
```
2022-03-10 15:14:55.780674 loconet_1 D2002: RX: OPC_IMM_PACKET [ED 0F 01 49 42 07 00 00 00 00 00 00 00 00 10]
2022-03-10 15:14:55.796607 loconet_1 D2002: RX: OPC_PEER_XFER [E5 0F 00 49 4B 09 08 11 00 00 03 06 00 00 02]

serial number = 1100008306
```

### Software version
```
2022-03-10 15:14:55.828622 loconet_1 D2002: RX: OPC_IMM_PACKET [ED 0F 01 49 42 06 00 00 00 00 00 00 00 00 11]
2022-03-10 15:14:55.844670 loconet_1 D2002: RX: OPC_PEER_XFER [E5 0F 00 49 4B 08 00 00 10 01 01 01 00 00 0E]

version = 1.00
```

## LISSY

68610: single sensor lissy format:
```
2022-10-09 22:45:43.034066 loconet_1 D2002: RX: E4 [E4 08 00 00 01 00 03 11] // sensor_address=1    decoder_address=3    category=1
2022-10-09 22:55:17.651995 loconet_1 D2002: RX: E4 [E4 08 03 00 01 00 63 72] // sensor_address=1    decoder_address=99   category=4
2022-10-09 23:03:55.056175 loconet_1 D2002: RX: E4 [E4 08 02 00 01 0F 50 4F] // sensor_address=1    decoder_address=2000 category=3
2022-10-09 23:09:03.150772 loconet_1 D2002: RX: E4 [E4 08 02 00 7F 0F 50 31] // sensor_address=127  decoder_address=2000 category=3
2022-10-09 23:12:03.391446 loconet_1 D2002: RX: E4 [E4 08 02 1F 7F 0F 50 2E] // sensor_address=4095 decoder_address=2000 category=3
                                                          ^^ ^^^^^ ^^^^^
                                                          |    |     \ decoder address: [high 7] [low 7]
                                                          |    |
                                                          |    \ sensor address: [high 5] [low 7]
                                                          |
                                                          \ category: 0-3 => 1-4
```

68610: double sensor lissy format:
```
2022-10-10 21:32:47.407984 loconet_1 D2002: RX: E4 [E4 08 02 60 01 0F 50 2F] // sensor_address=1 decoder_address=2000 category=3
2022-10-10 21:32:47.415238 loconet_1 D2002: RX: E4 [E4 08 00 20 01 00 17 25]

2022-10-10 21:34:57.571569 loconet_1 D2002: RX: E4 [E4 08 02 40 01 0F 50 0F] // sensor_address=1 decoder_address=2000 category=3 opposite_direction
2022-10-10 21:34:58.282593 loconet_1 D2002: RX: E4 [E4 08 00 20 01 00 04 36]

1st message simelar to single sensor, direction in 4th byte? 40 vs 60
2nd message speed? indicated by 4th byte 20? speed in km/h (last 2 data byte?)
```
