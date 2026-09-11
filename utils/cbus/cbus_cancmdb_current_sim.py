import can
import time
from random import randint

ACON2 = 0xD0
PRIO_LOW = 0x3 << 7
CAN_ID = 0x72
NODE = 0xFFFE
EVENT = 1


def hi(w):
    return (w >> 8) & 0xFF


def lo(w):
    return w & 0xFF


bus = can.interface.Bus(channel='vcan0', bustype='socketcan')

try:
    while True:
        current = randint(0, 20) * 50

        msg = can.Message(
            arbitration_id=PRIO_LOW | CAN_ID,
            data=[
                ACON2,
                hi(NODE), lo(NODE),
                hi(EVENT), lo(EVENT),
                hi(current), lo(current)],
            is_extended_id=False
        )

        bus.send(msg)
        print(f"{msg}")
        time.sleep(4)

except KeyboardInterrupt:
    pass
finally:
    bus.shutdown()
