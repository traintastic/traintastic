# LocoNet

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





