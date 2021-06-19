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

### LNCV write
```
LNCV  0 -> 5 : OPC_IMM_PACKET [ED 0F 01 05 00 20 01 4D 1A 00 00 05 00 00 6A]
LNCV  0 -> 1 : OPC_IMM_PACKET [ED 0F 01 05 00 20 01 4D 1A 00 00 01 00 00 6E]
LNCV  1 -> 2 : OPC_IMM_PACKET [ED 0F 01 05 00 20 01 4D 1A 01 00 02 00 00 6C]
LNCV  2 -> 1 : OPC_IMM_PACKET [ED 0F 01 05 00 20 01 4D 1A 02 00 01 00 00 6C]
LNCV 15 -> 1 : OPC_IMM_PACKET [ED 0F 01 05 00 20 01 4D 1A 0F 00 01 00 00 61]
                               ^^ ^^ ?? ?? ?? ?? ?? ?? ?? ^^ ^^^^^ ^^^^^ ^^
                              OPC  |                      LN value not   chk
                              IMM len                     CV u16   used? sum
                                                          u8 big
                                                             endian
```
