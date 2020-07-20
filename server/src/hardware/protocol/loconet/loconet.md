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





