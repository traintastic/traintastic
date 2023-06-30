/**
 * server/src/hardware/protocol/loconet/messages.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2023 Reinder Feenstra
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**
 * Portions Copyright (C) Digitrax Inc.
 *
 * LocoNet is a registered trademark of DigiTrax, Inc.
 */

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_MESSAGES_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_MESSAGES_HPP

#include <cstring>
#include <string>
#include <array>
#include <traintastic/enum/direction.hpp>
#include "../../../utils/byte.hpp"

// include all message headers:
#include "message/fastclock.hpp"
#include "message/immpacket.hpp"
#include "message/locof9f12imm.hpp"
#include "message/locof13f20imm.hpp"
#include "message/locof21f28imm.hpp"
#include "message/uhlenbrock.hpp"

namespace LocoNet {

bool isValid(const Message& message);

bool isLocoSlot(uint8_t slot);
void setSlot(Message& message, uint8_t slot);

bool hasResponse(const Message& message);
bool isValidResponse(const Message& request, const Message& response);

std::string toString(const Message& message);

constexpr uint8_t SLOT_DISPATCH = 0;
constexpr uint8_t SLOT_LOCO_MIN = 1;
constexpr uint8_t SLOT_LOCO_MAX = 119;
constexpr uint8_t SLOT_PROGRAMMING_TRACK = 124;
constexpr uint8_t SLOT_UNKNOWN = 255; //!< placeholder to indicate invalid slot

constexpr uint8_t SPEED_STOP = 0;
constexpr uint8_t SPEED_ESTOP = 1;
constexpr uint8_t SPEED_MIN = 2;
constexpr uint8_t SPEED_MAX = 127;

constexpr uint8_t SL_CONUP = 0x40;
constexpr uint8_t SL_BUSY = 0x20;
constexpr uint8_t SL_ACTIVE = 0x10;
constexpr uint8_t SL_CONDN = 0x08;

constexpr uint8_t SL_DIR = 0x20;
constexpr uint8_t SL_F0 = 0x10;
constexpr uint8_t SL_F4 = 0x08;
constexpr uint8_t SL_F3 = 0x04;
constexpr uint8_t SL_F2 = 0x02;
constexpr uint8_t SL_F1 = 0x01;

constexpr uint8_t SL_F5 = 0x01;
constexpr uint8_t SL_F6 = 0x02;
constexpr uint8_t SL_F7 = 0x04;
constexpr uint8_t SL_F8 = 0x08;

constexpr uint8_t SL_F9 = 0x01;
constexpr uint8_t SL_F10 = 0x02;
constexpr uint8_t SL_F11 = 0x04;
constexpr uint8_t SL_F12 = 0x08;

constexpr uint8_t SL_F13 = 0x01;
constexpr uint8_t SL_F14 = 0x02;
constexpr uint8_t SL_F15 = 0x04;
constexpr uint8_t SL_F16 = 0x08;
constexpr uint8_t SL_F17 = 0x10;
constexpr uint8_t SL_F18 = 0x20;
constexpr uint8_t SL_F19 = 0x40;

constexpr uint8_t SL_F21 = 0x01;
constexpr uint8_t SL_F22 = 0x02;
constexpr uint8_t SL_F23 = 0x04;
constexpr uint8_t SL_F24 = 0x08;
constexpr uint8_t SL_F25 = 0x10;
constexpr uint8_t SL_F26 = 0x20;
constexpr uint8_t SL_F27 = 0x40;

constexpr uint8_t SW2_ON = 0x10;
constexpr uint8_t SW2_DIR = 0x20;

constexpr uint8_t MULTI_SENSE_TYPE_MASK = 0xE0;
constexpr uint8_t MULTI_SENSE_TYPE_TRANSPONDER_GONE = 0x00;
constexpr uint8_t MULTI_SENSE_TYPE_TRANSPONDER_PRESENT = 0x20;
constexpr uint8_t MULTI_SENSE_TRANSPONDER_ADDRESS_SHORT = 0xFD;

struct SlotMessage : Message
{
  uint8_t slot;

  SlotMessage(OpCode _opCode, uint8_t _slot) :
    Message{_opCode},
    slot{_slot}
  {
  }
};

struct Idle : Message
{
  uint8_t checksum;

  Idle() :
    Message{OPC_IDLE},
    checksum{0x7A}
  {
  }
};
static_assert(sizeof(Idle) == 2);

struct GlobalPowerOn : Message
{
  uint8_t checksum;

  GlobalPowerOn() :
    Message{OPC_GPON},
    checksum{0x7C}
  {
  }
};
static_assert(sizeof(GlobalPowerOn) == 2);

struct GlobalPowerOff : Message
{
  uint8_t checksum;

  GlobalPowerOff() :
    Message{OPC_GPOFF},
    checksum{0x7D}
  {
  }
};
static_assert(sizeof(GlobalPowerOff) == 2);

struct Busy : Message
{
  uint8_t checksum;

  Busy() :
    Message{OPC_BUSY},
    checksum{0x7E}
  {
  }
};
static_assert(sizeof(Busy) == 2);



struct LocoAdr : Message
{
  uint8_t addressHigh;
  uint8_t addressLow;
  uint8_t checksum;

  LocoAdr(uint16_t address) :
    Message{OPC_LOCO_ADR},
    addressHigh{static_cast<uint8_t>(address >> 7)},
    addressLow{static_cast<uint8_t>(address & 0x7F)}
  {
    checksum = calcChecksum(*this);
  }

  uint16_t address() const
  {
    return (static_cast<uint16_t>(addressHigh) << 7) | addressLow;
  }
};
static_assert(sizeof(LocoAdr) == 4);

struct LocoSpd : SlotMessage
{
  uint8_t speed;
  uint8_t checksum;

  LocoSpd(uint8_t _speed) :
    SlotMessage{OPC_LOCO_SPD, SLOT_UNKNOWN},
    speed{_speed}
  {
    checksum = calcChecksum(*this);
  }
};
static_assert(sizeof(LocoSpd) == 4);

struct LocoDirF : SlotMessage
{
  uint8_t dirf;
  uint8_t checksum;

  LocoDirF(Direction direction, bool f0, bool f1, bool f2, bool f3, bool f4) :
    SlotMessage{OPC_LOCO_DIRF, SLOT_UNKNOWN},
    dirf{0}
  {
    assert(direction != Direction::Unknown);
    if(direction == Direction::Forward)
      dirf |= SL_DIR;
    if(f0)
      dirf |= SL_F0;
    if(f1)
      dirf |= SL_F1;
    if(f2)
      dirf |= SL_F2;
    if(f3)
      dirf |= SL_F3;
    if(f4)
      dirf |= SL_F4;

    checksum = calcChecksum(*this);
  }

  inline Direction direction() const
  {
    return (dirf & SL_DIR) ? Direction::Forward : Direction::Reverse;
  }

  inline void setDirection(Direction value)
  {
    assert(value != Direction::Unknown);
    if(value == Direction::Forward)
      dirf |= SL_DIR;
    else
      dirf &= ~SL_DIR;
  }

  bool f(uint8_t n) const
  {
    assert(n <= 5);
    return n == 0 ? f0() : ((dirf & (1 << (n - 1))) != 0);
  }

  inline bool f0() const
  {
    return dirf & SL_F0;
  }

  inline void setF0(bool value)
  {
    if(value)
      dirf |= SL_F0;
    else
      dirf &= ~SL_F0;
  }

  inline bool f1() const
  {
    return dirf & SL_F1;
  }

  inline void setF1(bool value)
  {
    if(value)
      dirf |= SL_F1;
    else
      dirf &= ~SL_F1;
  }

  inline bool f2() const
  {
    return dirf & SL_F2;
  }

  inline void setF2(bool value)
  {
    if(value)
      dirf |= SL_F2;
    else
      dirf &= ~SL_F2;
  }

  inline bool f3() const
  {
    return dirf & SL_F3;
  }

  inline void setF3(bool value)
  {
    if(value)
      dirf |= SL_F3;
    else
      dirf &= ~SL_F3;
  }

  inline bool f4() const
  {
    return dirf & SL_F4;
  }

  inline void setF4(bool value)
  {
    if(value)
      dirf |= SL_F4;
    else
      dirf &= ~SL_F4;
  }
};
static_assert(sizeof(LocoDirF) == 4);

struct LocoSnd : SlotMessage
{
  uint8_t snd;
  uint8_t checksum;

  LocoSnd(bool f5, bool f6, bool f7, bool f8) :
    SlotMessage{OPC_LOCO_SND, SLOT_UNKNOWN},
    snd{0}
  {
    if(f5)
      snd |= SL_F5;
    if(f6)
      snd |= SL_F6;
    if(f7)
      snd |= SL_F7;
    if(f8)
      snd |= SL_F8;

    checksum = calcChecksum(*this);
  }

  bool f(uint8_t n) const
  {
    assert(n >= 5 && n <= 8);
    return snd & (1 << (n - 5));
  }

  inline bool f5() const
  {
    return snd & SL_F5;
  }

  inline void setF5(bool value)
  {
    if(value)
      snd |= SL_F5;
    else
      snd &= ~SL_F5;
  }

  inline bool f6() const
  {
    return snd & SL_F6;
  }

  inline void setF6(bool value)
  {
    if(value)
      snd |= SL_F6;
    else
      snd &= ~SL_F6;
  }

  inline bool f7() const
  {
    return snd & SL_F7;
  }

  inline void setF7(bool value)
  {
    if(value)
      snd |= SL_F7;
    else
      snd &= ~SL_F7;
  }

  inline bool f8() const
  {
    return snd & SL_F8;
  }

  inline void setF8(bool value)
  {
    if(value)
      snd |= SL_F8;
    else
      snd &= ~SL_F8;
  }
};
static_assert(sizeof(LocoSnd) == 4);

struct LocoF9F12 : SlotMessage
{
  uint8_t function;
  uint8_t checksum;

  LocoF9F12(bool f9, bool f10, bool f11, bool f12) :
    SlotMessage{OPC_LOCO_F9F12, SLOT_UNKNOWN},
    function{0}
  {
    if(f9)
      function |= SL_F9;
    if(f10)
      function |= SL_F10;
    if(f11)
      function |= SL_F11;
    if(f12)
      function |= SL_F12;

    checksum = calcChecksum(*this);
  }

  bool f(uint8_t n) const
  {
    assert(n >= 9 && n <= 12);
    return function & (1 << (n - 9));
  }

  inline bool f9() const
  {
    return function & SL_F9;
  }

  inline void setF9(bool value)
  {
    if(value)
      function |= SL_F9;
    else
      function &= ~SL_F9;
  }

  inline bool f10() const
  {
    return function & SL_F10;
  }

  inline void setF10(bool value)
  {
    if(value)
      function |= SL_F10;
    else
      function &= ~SL_F10;
  }

  inline bool f11() const
  {
    return function & SL_F7;
  }

  inline void setF11(bool value)
  {
    if(value)
      function |= SL_F11;
    else
      function &= ~SL_F11;
  }

  inline bool f12() const
  {
    return function & SL_F8;
  }

  inline void setF12(bool value)
  {
    if(value)
      function |= SL_F12;
    else
      function &= ~SL_F12;
  }
};
static_assert(sizeof(LocoF9F12) == 4);

/*

2020-02-04 21:27:59.123954 [debug]    cs1: unknown message: dataLen=0x0008, header=0x00a0, data=b2 11 70 2c
2020-02-04 21:27:59.413558 [debug]    cs1: unknown message: dataLen=0x0008, header=0x00a0, data=b2 19 40 14
2020-02-04 21:28:00.046282 [debug]    cs1: unknown message: dataLen=0x0008, header=0x00a0, data=b2 11 50 0c
2020-02-04 21:28:00.433662 [debug]    cs1: unknown message: dataLen=0x0008, header=0x00a0, data=b2 11 60 3c
2020-02-04 21:28:02.502012 [debug]    cs1: unknown message: dataLen=0x0008, header=0x00a0, data=b2 09 70 34
2020-02-04 21:28:02.913595 [debug]    cs1: unknown message: dataLen=0x0008, header=0x00a0, data=b2 11 40 1c
2020-02-04 21:28:03.629638 [debug]    cs1: unknown message: dataLen=0x0008, header=0x00a0, data=b2 09 50 14
2020-02-04 21:28:03.937476 [debug]    cs1: unknown message: dataLen=0x0008, header=0x00a0, data=b2 09 60 24

  */



struct InputRep : Message
{
  static constexpr uint8_t control = 0x40;

  uint8_t in1;
  uint8_t in2;
  uint8_t checksum;

  InputRep() :
    Message{OPC_INPUT_REP}
  {
  }

  InputRep(uint16_t fullAddress_, bool value_) :
    InputRep{}
  {
    in1 = (fullAddress_ >> 1) & 0x7F;
    in2 = (fullAddress_ >> 8) & 0x0F;
    if(fullAddress_ & 0x0001)
      in2 |= 0x20;
    if(value_)
      in2 |= 0x10;
    in2 |= control; // set, 0 is reserved
    checksum = calcChecksum(*this);
  }

  inline uint16_t fullAddress() const
  {
    return (address() << 1) | (isSwitchInput() ? 1 : 0);
  }

  inline uint16_t address() const
  {
    return (in1 & 0x7F) | (static_cast<uint16_t>(in2 & 0x0F) << 7);
  }

  inline bool isControlSet() const
  {
    return in2 & control;
  }

  inline bool isSwitchInput() const
  {
    return in2 & 0x20;
  }

  inline bool isAuxInput() const
  {
    return !isSwitchInput();
  }

  inline bool value() const
  {
    return in2 & 0x10;
  }
};
static_assert(sizeof(InputRep) == 4);

struct LongAck : Message
{
  uint8_t lopc;
  uint8_t ack1;
  uint8_t checksum;

  LongAck(OpCode _lopc = static_cast<OpCode>(0), uint8_t _ack1 = 0) :
    Message{OPC_LONG_ACK},
    lopc{static_cast<uint8_t>(_lopc & 0x7F)},
    ack1{_ack1},
    checksum{calcChecksum(*this)}
  {
  }

  OpCode respondingOpCode() const
  {
    return static_cast<OpCode>(0x80 | lopc);
  }
};
static_assert(sizeof(LongAck) == 4);

struct SwitchRequest : Message
{
  uint8_t sw1;
  uint8_t sw2;
  uint8_t checksum;

  SwitchRequest() :
    Message{OPC_SW_REQ},
    sw1{0},
    sw2{0},
    checksum{0}
  {
  }

  SwitchRequest(uint16_t _address, bool _dir, bool _on) :
    SwitchRequest()
  {
    setAddress(_address);
    setDir(_dir);
    setOn(_on);
    updateChecksum(*this);
  }

  SwitchRequest(uint16_t _fullAddress, bool _on) :
    SwitchRequest()
  {
    setFullAddress(_fullAddress);
    setOn(_on);
    updateChecksum(*this);
  }

  inline uint16_t fullAddress() const
  {
    return (address() << 1) | (dir() ? 1 : 0);
  }

  inline void setFullAddress(uint16_t value)
  {
    setAddress(value >> 1);
    setDir(value & 0x1);
  }

  inline uint16_t address() const
  {
    return (sw1 & 0x7F) | (static_cast<uint16_t>(sw2 & 0x0F) << 7);
  }

  inline void setAddress(uint16_t value)
  {
    sw1 = value & 0x7F;
    sw2 = (sw2 & 0x30) | ((value >> 7) & 0x0F);
  }

  inline bool dir() const
  {
    return sw2 & SW2_DIR;
  }

  inline void setDir(bool value)
  {
    if(value)
      sw2 |= SW2_DIR;
    else
      sw2 &= ~SW2_DIR;
  }

  inline bool on() const
  {
    return sw2 & SW2_ON;
  }

  inline void setOn(bool value)
  {
    if(value)
      sw2 |= SW2_ON;
    else
      sw2 &= ~SW2_ON;
  }
};

struct RequestSlotData : Message
{
  uint8_t slot;
  uint8_t data2;
  uint8_t checksum;

  RequestSlotData(uint8_t _slot) :
    Message(OPC_RQ_SL_DATA),
    slot{_slot},
    data2{0}
  {
    checksum = calcChecksum(*this);
  }
};
static_assert(sizeof(RequestSlotData) == 4);


// d0 00 42 20 13 5e

struct MultiSense : Message
{
  uint8_t data1;
  uint8_t data2;
  uint8_t data3;
  uint8_t data4;
  uint8_t checksum;

  MultiSense() :
    Message(OPC_MULTI_SENSE)
  {
  }

  bool isTransponder() const
  {
    return
      ((data1 & MULTI_SENSE_TYPE_MASK) == MULTI_SENSE_TYPE_TRANSPONDER_GONE) ||
      ((data1 & MULTI_SENSE_TYPE_MASK) == MULTI_SENSE_TYPE_TRANSPONDER_PRESENT);
  }
};
static_assert(sizeof(MultiSense) == 6);

struct LocoF13F19 : Message
{
  uint8_t data1;
  uint8_t slot;
  uint8_t data3;
  uint8_t function;
  uint8_t checksum;

  LocoF13F19(bool f13, bool f14, bool f15, bool f16, bool f17, bool f18, bool f19) :
    Message(OPC_D4),
    data1{0x20},
    slot{SLOT_UNKNOWN},
    data3{0x08},
    function{0}
  {
    if(f13)
      function |= SL_F13;
    if(f14)
      function |= SL_F14;
    if(f15)
      function |= SL_F15;
    if(f16)
      function |= SL_F16;
    if(f17)
      function |= SL_F17;
    if(f18)
      function |= SL_F18;
    if(f19)
      function |= SL_F19;

    checksum = calcChecksum(*this);
  }

  bool f(uint8_t n) const
  {
    assert(n >= 13 && n <= 19);
    return (function & (1 << (n - 13)));
  }

  inline bool f13() const
  {
    return function & SL_F13;
  }

  inline void setF13(bool value)
  {
    if(value)
      function |= SL_F13;
    else
      function &= ~SL_F13;
  }

  inline bool f14() const
  {
    return function & SL_F14;
  }

  inline void setF14(bool value)
  {
    if(value)
      function |= SL_F14;
    else
      function &= ~SL_F14;
  }

  inline bool f15() const
  {
    return function & SL_F15;
  }

  inline void setF15(bool value)
  {
    if(value)
      function |= SL_F15;
    else
      function &= ~SL_F15;
  }

  inline bool f16() const
  {
    return function & SL_F16;
  }

  inline void setF16(bool value)
  {
    if(value)
      function |= SL_F16;
    else
      function &= ~SL_F16;
  }

  inline bool f17() const
  {
    return function & SL_F17;
  }

  inline void setF17(bool value)
  {
    if(value)
      function |= SL_F17;
    else
      function &= ~SL_F17;
  }

  inline bool f18() const
  {
    return function & SL_F18;
  }

  inline void setF18(bool value)
  {
    if(value)
      function |= SL_F18;
    else
      function &= ~SL_F18;
  }

  inline bool f19() const
  {
    return function & SL_F19;
  }

  inline void setF19(bool value)
  {
    if(value)
      function |= SL_F19;
    else
      function &= ~SL_F19;
  }
};
static_assert(sizeof(LocoF13F19) == 6);

struct LocoF12F20F28 : Message
{
  static constexpr uint8_t F12 = 0x10;
  static constexpr uint8_t F20 = 0x20;
  static constexpr uint8_t F28 = 0x40;

  uint8_t data1;
  uint8_t slot;
  uint8_t data3;
  uint8_t function;
  uint8_t checksum;

  LocoF12F20F28(bool f12, bool f20, bool f28) :
    Message(OPC_D4),
    data1{0x20},
    slot{SLOT_UNKNOWN},
    data3{0x05},
    function{0}
  {
    if(f12)
      function |= F12;
    if(f20)
      function |= F20;
    if(f28)
      function |= F28;

    checksum = calcChecksum(*this);
  }

  inline bool f12() const
  {
    return function & F12;
  }

  inline void setF12(bool value)
  {
    if(value)
      function |= F12;
    else
      function &= ~F12;
  }

  inline bool f20() const
  {
    return function & F20;
  }

  inline void setF20(bool value)
  {
    if(value)
      function |= F20;
    else
      function &= ~F20;
  }

  inline bool f28() const
  {
    return function & F28;
  }

  inline void setF28(bool value)
  {
    if(value)
      function |= F28;
    else
      function &= ~F28;
  }
};
static_assert(sizeof(LocoF12F20F28) == 6);

struct LocoF21F27 : Message
{
  uint8_t data1;
  uint8_t slot;
  uint8_t data3;
  uint8_t function;
  uint8_t checksum;

  LocoF21F27(bool f21, bool f22, bool f23, bool f24, bool f25, bool f26, bool f27) :
    Message(OPC_D4),
    data1{0x20},
    slot{SLOT_UNKNOWN},
    data3{0x09},
    function{0}
  {
    if(f21)
      function |= SL_F21;
    if(f22)
      function |= SL_F22;
    if(f23)
      function |= SL_F23;
    if(f24)
      function |= SL_F24;
    if(f25)
      function |= SL_F25;
    if(f26)
      function |= SL_F26;
    if(f27)
      function |= SL_F27;

    checksum = calcChecksum(*this);
  }

  bool f(uint8_t n) const
  {
    assert(n >= 21 && n <= 27);
    return (function & (1 << (n - 21)));
  }

  inline bool f21() const
  {
    return function & SL_F21;
  }

  inline void setF21(bool value)
  {
    if(value)
      function |= SL_F21;
    else
      function &= ~SL_F21;
  }

  inline bool f22() const
  {
    return function & SL_F22;
  }

  inline void setF22(bool value)
  {
    if(value)
      function |= SL_F22;
    else
      function &= ~SL_F22;
  }

  inline bool f23() const
  {
    return function & SL_F23;
  }

  inline void setF23(bool value)
  {
    if(value)
      function |= SL_F23;
    else
      function &= ~SL_F23;
  }

  inline bool f24() const
  {
    return function & SL_F24;
  }

  inline void setF24(bool value)
  {
    if(value)
      function |= SL_F24;
    else
      function &= ~SL_F24;
  }

  inline bool f25() const
  {
    return function & SL_F25;
  }

  inline void setF25(bool value)
  {
    if(value)
      function |= SL_F25;
    else
      function &= ~SL_F25;
  }

  inline bool f26() const
  {
    return function & SL_F26;
  }

  inline void setF26(bool value)
  {
    if(value)
      function |= SL_F26;
    else
      function &= ~SL_F26;
  }

  inline bool f27() const
  {
    return function & SL_F27;
  }

  inline void setF27(bool value)
  {
    if(value)
      function |= SL_F27;
    else
      function &= ~SL_F27;
  }
};
static_assert(sizeof(LocoF21F27) == 6);

struct MultiSenseTransponder : MultiSense
{
  bool isPresent() const
  {
    return (data1 & MULTI_SENSE_TYPE_MASK) == MULTI_SENSE_TYPE_TRANSPONDER_PRESENT;
  }

  uint16_t sensorAddress() const
  {
    return (static_cast<uint16_t>(data1 & 0x1F) << 7) | (data2 & 0x7F);
  }

  uint16_t transponderAddress() const
  {
    if(isTransponderAddressLong())
      return (static_cast<uint16_t>(data3 & 0x7F) << 7) | (data4 & 0x7F);
    else
      return (data4 & 0x7F);
  }

  bool isTransponderAddressLong() const
  {
    return data3 != MULTI_SENSE_TRANSPONDER_ADDRESS_SHORT;
  }
};
static_assert(sizeof(MultiSenseTransponder) == 6);

struct MultiSenseLong : Message
{
  uint8_t len;
  uint8_t data1;
  uint8_t data2;
  uint8_t data3;
  uint8_t data4;
  uint8_t data5;
  uint8_t data6;
  uint8_t checksum;

  MultiSenseLong() :
    Message(OPC_MULTI_SENSE_LONG),
    len{9}
  {
  }

  bool isTransponder() const
  {
    return
      ((data1 & MULTI_SENSE_TYPE_MASK) == MULTI_SENSE_TYPE_TRANSPONDER_GONE) ||
      ((data1 & MULTI_SENSE_TYPE_MASK) == MULTI_SENSE_TYPE_TRANSPONDER_PRESENT);
  }
};
static_assert(sizeof(MultiSenseLong) == 9);

struct MultiSenseLongTransponder : MultiSenseLong
{
  bool isPresent() const
  {
    return (data1 & MULTI_SENSE_TYPE_MASK) == MULTI_SENSE_TYPE_TRANSPONDER_PRESENT;
  }

  uint16_t sensorAddress() const
  {
    return (static_cast<uint16_t>(data1 & 0x1F) << 7) | (data2 & 0x7F);
  }

  uint16_t transponderAddress() const
  {
    if(isTransponderAddressLong())
      return (static_cast<uint16_t>(data3 & 0x7F) << 7) | (data4 & 0x7F);
    else
      return (data4 & 0x7F);
  }

  bool isTransponderAddressLong() const
  {
    return data3 != MULTI_SENSE_TRANSPONDER_ADDRESS_SHORT;
  }

  Direction transponderDirection() const
  {
    return (data5 & 0x40) ? Direction::Forward : Direction::Reverse;
  }
};
static_assert(sizeof(MultiSenseLongTransponder) == 9);

struct SlotReadDataBase : Message
{
  uint8_t len;
  uint8_t slot;

  SlotReadDataBase(uint8_t _slot = 0)
    : Message(OPC_SL_RD_DATA)
    , len{14}
    , slot{_slot}
  {
  }
};

struct SlotReadData : SlotReadDataBase
{
  uint8_t stat = 0;
  uint8_t adr = 0;
  uint8_t spd = 0;
  uint8_t dirf = 0;
  uint8_t trk = 0;
  uint8_t ss2 = 0;
  uint8_t adr2 = 0;
  uint8_t snd = 0;
  uint8_t id1 = 0;
  uint8_t id2 = 0;
  uint8_t checksum;

  SlotReadData(uint8_t _slot = 0)
    : SlotReadDataBase(_slot)
    , checksum{calcChecksum(*this)}
  {
  }

  bool isBusy() const
  {
    return stat & SL_BUSY;
  }

  void setBusy(bool value)
  {
    if(value)
      stat |= SL_BUSY;
    else
      stat &= ~SL_BUSY;
  }

  bool isActive() const
  {
    return stat & SL_ACTIVE;
  }

  void setActive(bool value)
  {
    if(value)
      stat |= SL_ACTIVE;
    else
      stat &= ~SL_ACTIVE;
  }

  bool isFree() const
  {
    return !isBusy() && !isActive();
  }

  uint16_t address() const
  {
    return (static_cast<uint16_t>(adr2) << 7) | adr;
  }

  void setAddress(uint16_t value)
  {
    adr = value & 0x7F;
    adr2 = (value >> 7) & 0x7F;
  }

  bool isEmergencyStop() const
  {
    return spd == 0x01;
  }

  uint8_t speed() const
  {
    return spd > 1 ? spd - 1 : 0;
  }

  inline Direction direction() const
  {
    return (dirf & SL_DIR) ? Direction::Forward : Direction::Reverse;
  }

  inline void setDirection(Direction value)
  {
    if(value == Direction::Forward)
      dirf |= SL_DIR;
    else
      dirf &= ~SL_DIR;
  }

  bool f(uint8_t n) const
  {
    assert(n <= 8);
    if(n == 0)
      return f0();
    else if(n <= 4)
      return (dirf & (1 << (n - 1)));
    else // n <= 8
      return (snd & (1 << (n - 5)));
  }

  inline bool f0() const
  {
    return dirf & SL_F0;
  }

  inline void setF0(bool value)
  {
    if(value)
      dirf |= SL_F0;
    else
      dirf &= ~SL_F0;
  }

  inline bool f1() const
  {
    return dirf & SL_F1;
  }

  inline void setF1(bool value)
  {
    if(value)
      dirf |= SL_F1;
    else
      dirf &= ~SL_F1;
  }

  inline bool f2() const
  {
    return dirf & SL_F2;
  }

  inline void setF2(bool value)
  {
    if(value)
      dirf |= SL_F2;
    else
      dirf &= ~SL_F2;
  }

  inline bool f3() const
  {
    return dirf & SL_F3;
  }

  inline void setF3(bool value)
  {
    if(value)
      dirf |= SL_F3;
    else
      dirf &= ~SL_F3;
  }

  inline bool f4() const
  {
    return dirf & SL_F4;
  }

  inline void setF4(bool value)
  {
    if(value)
      dirf |= SL_F4;
    else
      dirf &= ~SL_F4;
  }

  inline bool f5() const
  {
    return snd & SL_F5;
  }

  inline void setF5(bool value)
  {
    if(value)
      snd |= SL_F5;
    else
      snd &= ~SL_F5;
  }

  inline bool f6() const
  {
    return snd & SL_F6;
  }

  inline void setF6(bool value)
  {
    if(value)
      snd |= SL_F6;
    else
      snd &= ~SL_F6;
  }

  inline bool f7() const
  {
    return snd & SL_F7;
  }

  inline void setF7(bool value)
  {
    if(value)
      snd |= SL_F7;
    else
      snd &= ~SL_F7;
  }

  inline bool f8() const
  {
    return snd & SL_F8;
  }

  inline void setF8(bool value)
  {
    if(value)
      snd |= SL_F8;
    else
      snd &= ~SL_F8;
  }
};
static_assert(sizeof(SlotReadData) == 14);

/*
struct ImmediatePacket : Message
{
  uint8_t len;
  uint8_t header;
  uint8_t reps;
  uint8_t dhi;
  uint8_t im[5];
  uint8_t checksum;

  ImmediatePacket() :
    Message(OPC_IMM_PACKET),
    len{11},
    header{0x7F},
    reps{0},
    dhi{0},
    im{0, 0, 0, 0, 0}
  {
  }

  void setIMCount(uint8_t value)
  {
    assert(value <= 5);
    reps = (reps & 0x8F) | ((value & 0x07) << 4);
  }

  void updateDHI()
  {
    dhi = 0x20 |
      (im[0] & 0x40) >> 7 |
      (im[1] & 0x40) >> 6 |
      (im[2] & 0x40) >> 5 |
      (im[3] & 0x40) >> 4 |
      (im[4] & 0x40) >> 3;
  }
};
static_assert(sizeof(ImmediatePacket) == 11);

struct ImmediatePacketLoco : ImmediatePacket
{
  ImmediatePacketLoco(uint16_t address, uint8_t repeatCount) :
    ImmediatePacket()
  {
    assert(repeatCount <= 7);
    reps = repeatCount & 0x07;

    if(isLongAddress(address))
    {
      im[0] = 0xC0 | ((address >> 8) & 0x3F);
      im[1] = address & 0xFF;
    }
    else
      im[0] = address & 0x7F;
  }
};
static_assert(sizeof(ImmediatePacketLoco) == sizeof(ImmediatePacket));

struct ImmediatePacketF9F12 : ImmediatePacketLoco
{
  ImmediatePacketF9F12(uint16_t address, bool f9, bool f10, bool f11, bool f12, uint8_t repeatCount = 2) :
    ImmediatePacketLoco(address, repeatCount)
  {
    const uint8_t offset = (im[0] & 0x80) ? 2 : 1;

    im[offset] = 0xB0; // Function group two instruction: F9-F12
    if(f9)
      im[offset] |= 0x01;
    if(f10)
      im[offset] |= 0x02;
    if(f11)
      im[offset] |= 0x04;
    if(f12)
      im[offset] |= 0x08;

    setIMCount(offset + 1);
    updateDHI();
    checksum = calcChecksum(*this);
  }
};
static_assert(sizeof(ImmediatePacketF9F12) == sizeof(ImmediatePacketLoco));

struct ImmediatePacketF13F20 : ImmediatePacketLoco
{
  ImmediatePacketF13F20(uint16_t address, bool f13, bool f14, bool f15, bool f16, bool f17, bool f18, bool f19, bool f20, uint8_t repeatCount = 2) :
    ImmediatePacketLoco(address, repeatCount)
  {
    uint8_t offset = (im[0] & 0x80) ? 2 : 1;

    im[offset++] = 0xDE; // Feature Expansion Instruction: F13-F20 function control

    if(f13)
      im[offset] |= 0x01;
    if(f14)
      im[offset] |= 0x02;
    if(f15)
      im[offset] |= 0x04;
    if(f16)
      im[offset] |= 0x08;
    if(f17)
      im[offset] |= 0x10;
    if(f18)
      im[offset] |= 0x20;
    if(f19)
      im[offset] |= 0x40;
    if(f20)
      im[offset] |= 0x80;

    setIMCount(offset + 1);
    updateDHI();
    checksum = calcChecksum(*this);
  }
};
static_assert(sizeof(ImmediatePacketF13F20) == sizeof(ImmediatePacketLoco));

struct ImmediatePacketF21F28 : ImmediatePacketLoco
{
  ImmediatePacketF21F28(uint16_t address, bool f21, bool f22, bool f23, bool f24, bool f25, bool f26, bool f27, bool f28, uint8_t repeatCount = 2) :
    ImmediatePacketLoco(address, repeatCount)
  {
    uint8_t offset = (im[0] & 0x80) ? 2 : 1;

    im[offset++] = 0xDF; // Feature Expansion Instruction: F20-F28 function control

    if(f21)
      im[offset] |= 0x01;
    if(f22)
      im[offset] |= 0x02;
    if(f23)
      im[offset] |= 0x04;
    if(f24)
      im[offset] |= 0x08;
    if(f25)
      im[offset] |= 0x10;
    if(f26)
      im[offset] |= 0x20;
    if(f27)
      im[offset] |= 0x40;
    if(f28)
      im[offset] |= 0x80;

    setIMCount(offset + 1);
    updateDHI();
    checksum = calcChecksum(*this);
  }
};
static_assert(sizeof(ImmediatePacketF21F28) == sizeof(ImmediatePacketLoco));
*/

}

inline bool operator ==(const LocoNet::Message& lhs, const LocoNet::Message& rhs)
{
  return lhs.size() == rhs.size() && std::memcmp(&lhs, &rhs, lhs.size()) == 0;
}

#endif
