/**
 * server/src/hardware/protocol/z21/messages.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_Z21_MESSAGES_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_Z21_MESSAGES_HPP

#include <cstdint>
#include <cstring>
#include <string>
#include <traintastic/enum/direction.hpp>
#include "utils.hpp"
#include "../../../utils/packed.hpp"
#include "../../../utils/endian.hpp"

class Decoder;

namespace Z21 {

struct Message;

std::string toString(const Message& message, bool raw = false);

enum Header : uint16_t
{
  LAN_GET_SERIAL_NUMBER = 0x10,
  LAN_GET_HWINFO = 0x1A,
  LAN_LOGOFF = 0x30,
  LAN_X = 0x40,
  LAN_SET_BROADCASTFLAGS = 0x50,
  LAN_GET_BROADCASTFLAGS = 0x51,
  LAN_GET_LOCO_MODE = 0x60,
  LAN_SET_LOCO_MODE = 0x61,
  LAN_SYSTEMSTATE_DATACHANGED = 0x84,
  LAN_SYSTEMSTATE_GETDATA = 0x85,
  LAN_LOCONET_Z21_RX = 0xA0,
  LAN_LOCONET_Z21_TX = 0xA1,
};

enum BroadcastFlags : uint32_t
{
  /**
   * Broadcasts and info messages concerning driving and switching are delivered to the registered clients automatically.
   * The following messages are concerned:
   * 2.7 LAN_X_BC_TRACK_POWER_OFF
   * 2.8 LAN_X_BC_TRACK_POWER_ON
   * 2.9 LAN_X_BC_PROGRAMMING_MODE
   * 2.10 LAN_X_BC_TRACK_SHORT_CIRCUIT
   * 2.14 LAN_X_BC_STOPPED
   * 4.4 LAN_X_LOCO_INFO (loco address must be subscribed too)
   * 5.3 LAN_X_TURNOUT_INFO
   */
  PowerLocoTurnout = 0x00000001,
};

enum LocoMode : uint8_t
{
  DCC = 0,
  Motorola = 1,
};

static constexpr uint8_t LAN_X_SET_STOP = 0x80;
//static constexpr uint8_t LAN_X_TURNOUT_INFO = 0x43;
static constexpr uint8_t LAN_X_BC = 0x61;
static constexpr uint8_t LAN_X_BC_TRACK_POWER_OFF = 0x00;
static constexpr uint8_t LAN_X_BC_TRACK_POWER_ON = 0x01;
//static constexpr uint8_t LAN_X_BC_PROGRAMMING_MODE = 0x02;
//static constexpr uint8_t LAN_X_BC_TRACK_SHORT_CIRCUIT = 0x08;
//static constexpr uint8_t LAN_X_CV_NACK_SC = 0x12;
//static constexpr uint8_t LAN_X_CV_NACK = 0x13;
//static constexpr uint8_t LAN_X_UNKNOWN_COMMAND = 0x82;
static constexpr uint8_t LAN_X_BC_STOPPED = 0x81;
static constexpr uint8_t LAN_X_LOCO_INFO = 0xEF;

enum HardwareType : uint32_t
{
  HWT_Z21_OLD = 0x00000200, //!< „black Z21” (hardware variant from 2012)
  HWT_Z21_NEW = 0x00000201, //!<  „black Z21”(hardware variant from 2013)
  HWT_SMARTRAIL = 0x00000202, //!< SmartRail (from 2012)
  HWT_Z21_SMALL = 0x00000203, //!< „white z21” starter set variant (from 2013)
  HWT_Z21_START = 0x00000204, //!< „z21 start” starter set variant (from 2016)
};

#define Z21_CENTRALSTATE_EMERGENCYSTOP 0x01 //!< The emergency stop is switched on
#define Z21_CENTRALSTATE_TRACKVOLTAGEOFF 0x02 //!< The track voltage is switched off
#define Z21_CENTRALSTATE_SHORTCIRCUIT 0x04 //!< Short circuit
#define Z21_CENTRALSTATE_PROGRAMMINGMODEACTIVE 0x20 //!< The programming mode is active

#define Z21_CENTRALSTATEEX_HIGHTEMPERATURE 0x01 //!< Temperature too high
#define Z21_CENTRALSTATEEX_POWERLOST 0x02 //!< Input voltage too low
#define Z21_CENTRALSTATEEX_SHORTCIRCUITEXTERNAL 0x04 //!< Short circuit at the external booster output
#define Z21_CENTRALSTATEEX_SHORTCIRCUITINTERNAL 0x08 //!< Short circuit at the main track or programming track

PRAGMA_PACK_PUSH_1

struct Message
{
  uint16_t dataLenLE; //!< DataLen (little endian): Total length over the entire data set including DataLen, Header and Data, i.e. DataLen = 2+2+n.
  Header headerLE;  //!< Header (little endian): Describes the Command and the Protocol’s group. \see Header

  Message(uint16_t _dataLen, Header _header) :
    dataLenLE{host_to_le(_dataLen)},
    headerLE{host_to_le(_header)}
  {
  }

  inline uint16_t dataLen() const
  {
    return le_to_host(dataLenLE);
  }

  inline Header header() const
  {
    return static_cast<Header>(le_to_host(headerLE));
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(Message) == 4);

struct LanX : Message
{
  uint8_t xheader;

  LanX(uint16_t _dataLen, uint8_t _xheader) :
    Message(_dataLen, LAN_X),
    xheader{_xheader}
  {
  }

  void calcChecksum(uint8_t len)
  {
    uint8_t* checksum = &xheader + len + 1;
    *checksum = xheader;
    for(uint8_t* db = &xheader + 1; db < checksum; db++)
      *checksum ^= *db;
  }

  inline void calcChecksum()
  {
    calcChecksum(xheader & 0x0F);
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanX) == 5);

//=============================================================================
// Client to Z21

// LAN_GET_SERIAL_NUMBER
struct LanGetSerialNumber : Message
{
  LanGetSerialNumber() :
    Message(sizeof(LanGetSerialNumber), LAN_GET_SERIAL_NUMBER)
  {
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanGetSerialNumber) == 4);

// LAN_GET_CODE

// LAN_GET_HWINFO
struct LanGetHardwareInfo : Message
{
  LanGetHardwareInfo() :
    Message(sizeof(LanGetHardwareInfo), LAN_GET_HWINFO)
  {
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanGetHardwareInfo) == 4);

// LAN_LOGOFF
struct LanLogoff : Message
{
  LanLogoff() :
    Message(sizeof(LanLogoff), LAN_LOGOFF)
  {
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanLogoff) == 4);

// LAN_X_GET_VERSION
struct LanXGetFirmwareVersion : LanX
{
  uint8_t db0 = 0x0A;
  uint8_t checksum = 0xFB;

  LanXGetFirmwareVersion() :
    LanX(sizeof(LanXGetFirmwareVersion), 0xF1)
  {
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanXGetFirmwareVersion) == 7);

// LAN_X_GET_STATUS
struct LanXGetStatus : LanX
{
  uint8_t db0 = 0x24;
  uint8_t checksum = 0x05;

  LanXGetStatus() :
    LanX(sizeof(LanXGetStatus), 0x21)
  {
  }
};
static_assert(sizeof(LanXGetStatus) == 7);

// LAN_X_SET_TRACK_POWER_OFF
struct LanXSetTrackPowerOff : LanX
{
  uint8_t db0 = 0x80;
  uint8_t checksum = 0xa1;

  LanXSetTrackPowerOff() :
    LanX(sizeof(LanXSetTrackPowerOff), 0x21)
  {
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanXSetTrackPowerOff) == 7);

// LAN_X_SET_TRACK_POWER_ON
struct LanXSetTrackPowerOn : LanX
{
  uint8_t db0 = 0x81;
  uint8_t checksum = 0xa0;

  LanXSetTrackPowerOn() :
    LanX(sizeof(LanXSetTrackPowerOn), 0x21)
  {
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanXSetTrackPowerOn) == 7);

// LAN_X_DCC_READ_REGISTER

// LAN_X_CV_READ

// LAN_X_DCC_WRITE_REGISTER

// LAN_X_CV_WRITE

// LAN_X_MWRITE_BYTE

// LAN_X_GET_TURNOUT_INFO

// LAN_X_SET_TURNOUT

// LAN_X_SET_STOP
struct LanXSetStop : LanX
{
  uint8_t checksum = 0x80;

  LanXSetStop() :
    LanX(sizeof(LanXSetStop), LAN_X_SET_STOP)
  {
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanXSetStop) == 6);

// LAN_X_GET_LOCO_INFO
struct LanXGetLocoInfo : LanX
{
  uint8_t db0 = 0xF0;
  uint8_t addressHigh;
  uint8_t addressLow;
  uint8_t checksum;

  LanXGetLocoInfo(uint16_t address, bool longAddress) :
    LanX(sizeof(LanXGetLocoInfo),  0xE3)
  {
    setAddress(address, longAddress);
    calcChecksum();
  }

  inline uint16_t address() const
  {
    return (static_cast<uint16_t>(addressHigh & 0x3F) << 8) | addressLow;
  }

  inline bool isLongAddress() const
  {
    return (addressHigh & 0xC0) == 0xC0;
  }

  inline void setAddress(uint16_t address, bool longAddress)
  {
    addressHigh = longAddress ? (0xC0 | (address >> 8)) : 0x00;
    addressLow = longAddress ? address & 0xFF : address & 0x7F;
  }

  /*inline void calcChecksum()
  {
    checksum = xheader ^ db0 ^ addressHigh ^ addressLow;
  }*/
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanXGetLocoInfo) == 9);

// LAN_X_SET_LOCO_DRIVE
struct LanXSetLocoDrive : LanX
{
  //static constexpr uint8_t directionFlag = 0x80;

  //uint8_t xheader = 0xe4;
  uint8_t db0;
  uint8_t addressHigh;
  uint8_t addressLow;
  uint8_t speedAndDirection = 0;
  uint8_t checksum;

  LanXSetLocoDrive() :
    LanX(sizeof(LanXSetLocoDrive), 0xE4)
  {
  }

  inline uint16_t address() const
  {
    return (static_cast<uint16_t>(addressHigh & 0x3F) << 8) | addressLow;
  }

  inline bool isLongAddress() const
  {
    return (addressHigh & 0xC0) == 0xC0;
  }

  inline uint8_t speedSteps() const
  {
    switch(db0 & 0x0F)
    {
      case 0: return 14;
      case 2: return 28;
      case 3: return 126;
      default: return 0;
    }
  }

  inline Direction direction() const
  {
    return Utils::getDirection(speedAndDirection);
  }

  inline void setDirection(Direction value)
  {
    Utils::setDirection(speedAndDirection, value);
  }

  inline bool isEmergencyStop() const
  {
    return Utils::isEmergencyStop(speedAndDirection, speedSteps());
  }

  inline void setEmergencyStop()
  {
    Utils::setEmergencyStop(speedAndDirection);
  }

  inline uint8_t speedStep() const
  {
    return Utils::getSpeedStep(speedAndDirection, speedSteps());
  }

  inline void setSpeedStep(uint8_t value)
  {
    Utils::setSpeedStep(speedAndDirection, speedSteps(), value);
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanXSetLocoDrive) == 10);

// LAN_X_SET_LOCO_FUNCTION
struct LanXSetLocoFunction : LanX
{
  enum class SwitchType
  {
    Off = 0,
    On = 1,
    Toggle = 2,
    Invalid = 3,
  };

  uint8_t db0 = 0xf8;
  uint8_t addressHigh;
  uint8_t addressLow;
  uint8_t db3;
  uint8_t checksum;

  LanXSetLocoFunction() :
    LanX(sizeof(LanXSetLocoFunction), 0xE4)
  {
  }

  inline uint16_t address() const
  {
    return (static_cast<uint16_t>(addressHigh & 0x3F) << 8) | addressLow;
  }

  inline bool isLongAddress() const
  {
    return (addressHigh & 0xC0) == 0xC0;
  }

  inline SwitchType switchType() const
  {
    return static_cast<SwitchType>(db3 >> 6);
  }

  inline uint8_t functionIndex() const
  {
    return db3 & 0x3F;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanXSetLocoFunction) == 10);

// LAN_X_CV_POWRITE_BYTE

// LAN_X_CV_POWRITE_BIT

// LAN_X_CV_POREAD_BYTE

// LAN_X_CV_POACCESSORY_WRITE_BYTE

// LAN_X_CV_PO ACCESSORY_WRITE_BIT

// LAN_X_CV_PO ACCESSORY_READ_BYTE

// LAN_X_GET_FIRMWARE_VERSION

// LAN_SET_BROADCASTFLAGS
struct LanSetBroadcastFlags : Message
{
  BroadcastFlags broadcastFlagsLE; // LE

  LanSetBroadcastFlags(uint32_t _broadcastFlags = 0) :
    Message(sizeof(LanSetBroadcastFlags), LAN_SET_BROADCASTFLAGS),
    broadcastFlagsLE{host_to_le(_broadcastFlags)}
  {
  }

  inline BroadcastFlags broadcastFlags() const
  {
    return le_to_host(broadcastFlagsLE);
  }

} ATTRIBUTE_PACKED;
static_assert(sizeof(LanSetBroadcastFlags) == 8);

// LAN_GET_BROADCASTFLAGS
struct LanGetBroadcastFlags : Message
{
  LanGetBroadcastFlags() :
    Message(sizeof(LanGetBroadcastFlags), LAN_GET_BROADCASTFLAGS)
  {
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanGetBroadcastFlags) == 4);

// LAN_GET_LOCOMODE
struct LanGetLocoMode : Message
{
  uint16_t addressBE; // BE

  LanGetLocoMode(uint16_t _address = 0) :
    Message(sizeof(LanGetLocoMode), LAN_GET_LOCO_MODE)
  {
    setAddress(_address);
  }

  inline uint16_t address() const
  {
    return be_to_host(addressBE);
  }

  inline void setAddress(uint16_t value)
  {
    addressBE = host_to_be(value);
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanGetLocoMode) == 6);

// LAN_SET_LOCOMODE
struct LanSetLocoMode : Message
{
  uint16_t addressBE;
  LocoMode mode;

  LanSetLocoMode(uint16_t _address, LocoMode _mode) :
    Message(sizeof(LanSetLocoMode), LAN_SET_LOCO_MODE),
    addressBE{host_to_be(_address)},
    mode{_mode}
  {
  }

  inline uint16_t address() const
  {
    return be_to_host(addressBE);
  }

  inline void setAddress(uint16_t value)
  {
    addressBE = host_to_be(value);
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanSetLocoMode) == 7);

// LAN_GET_TURNOUTMODE

// LAN_SET_TURNOUTMODE

// LAN_RMBUS_GETDATA

// LAN_RMBUS_PROGRAMMODULE

// LAN_SYSTEMSTATE_GETDATA
struct LanSystemStateGetData : Message
{
  LanSystemStateGetData() :
    Message(sizeof(LanSystemStateGetData), LAN_SYSTEMSTATE_GETDATA)
  {
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanSystemStateGetData) == 4);

// LAN_RAILCOGETDATA

// LAN_LOCONET_FROLAN

// LAN_LOCONET_DISPATCH_ADDR

// LAN_LOCONET_DETECTOR

// LAN_CAN_DETECTOR

//=============================================================================
// Z21 to Client:

// Reply to LAN_GET_SERIAL_NUMBER
struct LanGetSerialNumberReply : Message
{
  uint32_t serialNumberLE;

  LanGetSerialNumberReply(uint32_t _serialNumber) :
    Message(sizeof(LanGetSerialNumberReply), LAN_GET_SERIAL_NUMBER)
  {
    setSerialNumber(_serialNumber);
  }

  inline uint32_t serialNumber() const
  {
    return le_to_host(serialNumberLE);
  }

  inline void setSerialNumber(uint32_t value)
  {
    serialNumberLE = host_to_le(value);
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanGetSerialNumberReply) == 8);

// Reply to LAN_GET_CODE
struct LanXGetFirmwareVersionReply : LanX
{
  uint8_t db0 = 0x0A;
  uint8_t majorBCD;
  uint8_t minorBCD;
  uint8_t checksum;

  LanXGetFirmwareVersionReply() :
    LanX(sizeof(LanXGetFirmwareVersionReply), 0xF3)
  {
  }

  LanXGetFirmwareVersionReply(uint8_t _major, uint8_t _minor) :
    LanXGetFirmwareVersionReply()
  {
    setVersionMajor(_major);
    setVersionMinor(_minor);
    calcChecksum();
  }

  inline uint8_t versionMajor() const
  {
    return Utils::fromBCD(majorBCD);
  }

  inline uint8_t versionMinor() const
  {
    return Utils::fromBCD(minorBCD);
  }

  inline void setVersionMajor(uint8_t value)
  {
    assert(value < 100);
    majorBCD = Utils::toBCD(value);
  }

  inline void setVersionMinor(uint8_t value)
  {
    assert(value < 100);
    minorBCD = Utils::toBCD(value);
  }
};
static_assert(sizeof(LanXGetFirmwareVersionReply) == 9);

// Reply to LAN_GET_HWINFO
struct LanGetHardwareInfoReply : Message
{
  HardwareType hardwareTypeLE;
  uint32_t firmwareVersionLE;

  LanGetHardwareInfoReply(HardwareType _hardwareType, uint8_t _firmwareVersionMajor, uint8_t _firmwareVersionMinor) :
    Message(sizeof(LanGetHardwareInfoReply), LAN_GET_HWINFO),
    hardwareTypeLE{host_to_le(_hardwareType)},
    firmwareVersionLE{host_to_le(static_cast<uint32_t>(Z21::Utils::toBCD(_firmwareVersionMajor)) << 8 | Z21::Utils::toBCD(_firmwareVersionMinor))}
  {
  }

  HardwareType hardwareType() const
  {
    return le_to_host(hardwareTypeLE);
  }

  uint8_t firmwareVersionMajor() const
  {
    return Utils::fromBCD((le_to_host(firmwareVersionLE) >> 8) && 0xFF);
  }

  uint8_t firmwareVersionMinor() const
  {
    return Utils::fromBCD(le_to_host(firmwareVersionLE) && 0xFF);
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanGetHardwareInfoReply) == 12);

// LAN_X_TURNOUT_INFO

// LAN_X_BC_TRACK_POWER_OFF
struct LanXBCTrackPowerOff : LanX
{
  uint8_t db0 = LAN_X_BC_TRACK_POWER_OFF;
  uint8_t checksum = 0x61;

  LanXBCTrackPowerOff() :
    LanX(sizeof(LanXBCTrackPowerOff), LAN_X_BC)
  {
  }
};
static_assert(sizeof(LanXBCTrackPowerOff) == 7);

// LAN_X_BC_TRACK_POWER_ON
struct LanXBCTrackPowerOn : LanX
{
  uint8_t db0 = LAN_X_BC_TRACK_POWER_ON;
  uint8_t checksum = 0x60;

  LanXBCTrackPowerOn() :
    LanX(sizeof(LanXBCTrackPowerOn), LAN_X_BC)
  {
  }
};
static_assert(sizeof(LanXBCTrackPowerOn) == 7);

// LAN_X_BC_PROGRAMMING_MODE

// LAN_X_BC_TRACK_SHORT_CIRCUIT

// LAN_X_CV_NACK_SC

// LAN_X_CV_NACK

// LAN_X_UNKNOWN_COMMAND

// LAN_X_STATUS_CHANGED
struct LanXStatusChanged : LanX
{
  uint8_t db0 = 0x22;
  uint8_t db1 = 0;
  uint8_t checksum;

  LanXStatusChanged() :
    LanX(sizeof(LanXStatusChanged), 0x62)
  {
  }
};
static_assert(sizeof(LanXStatusChanged) == 8);

// Reply to LAN_X_GET_VERSION

// LAN_X_CV_RESULT

// LAN_X_BC_STOPPED
struct LanXBCStopped : LanX
{
  uint8_t db0 = 0x00;
  uint8_t checksum = 0x80;

  LanXBCStopped() :
    LanX(sizeof(LanXBCStopped), LAN_X_BC_STOPPED)
  {
  }
};
static_assert(sizeof(LanXBCStopped) == 7);

// LAN_X_LOCO_INFO
struct LanXLocoInfo : LanX
{
  static constexpr uint8_t db2_busy_flag = 0x08;
  static constexpr uint8_t db2_speed_steps_14 = 0x00;
  static constexpr uint8_t db2_speed_steps_28 = 0x02;
  static constexpr uint8_t db2_speed_steps_128 = 0x04;
  static constexpr uint8_t db2_speed_steps_mask = 0x07;
  static constexpr uint8_t directionFlag = 0x80;
  static constexpr uint8_t speedStepMask = 0x7F;
  static constexpr uint8_t flagF0 = 0x10;

  uint8_t addressHigh = 0;
  uint8_t addressLow = 0;
  uint8_t db2 = 0;
  uint8_t speedAndDirection = 0;
  uint8_t db4 = 0;
  uint8_t f5f12 = 0;
  uint8_t f13f20 = 0;
  uint8_t f21f28 = 0;
  uint8_t checksum = 0;

  LanXLocoInfo() :
    LanX(sizeof(LanXLocoInfo), LAN_X_LOCO_INFO)
  {
  }

  LanXLocoInfo(const Decoder& decoder);

  inline uint16_t address() const
  {
    return (static_cast<uint16_t>(addressHigh & 0x3F) << 8) | addressLow;
  }

  inline bool isLongAddress() const
  {
    return (addressHigh & 0xC0) == 0xC0;
  }

  inline void setAddress(uint16_t address, bool longAddress)
  {
    addressHigh = longAddress ? (0xC0 | (address >> 8)) : 0x00;
    addressLow = longAddress ? address & 0xFF : address & 0x7F;
  }

  inline bool isBusy() const
  {
    return db2 & db2_busy_flag;
  }

  inline void setBusy(bool value)
  {
    if(value)
      db2 |= db2_busy_flag;
    else
      db2 &= ~db2_busy_flag;
  }

  inline uint8_t speedSteps() const
  {
    switch(db2 & db2_speed_steps_mask)
    {
      case db2_speed_steps_14:  return 14;
      case db2_speed_steps_28:  return 28;
      case db2_speed_steps_128: return 126;
    }
    return 0;
  }

  inline void setSpeedSteps(uint8_t value)
  {
    db2 &= ~db2_speed_steps_mask;
    switch(value)
    {
      case 14:  db2 |= db2_speed_steps_14;  break;
      case 28:  db2 |= db2_speed_steps_28;  break;
      case 126: db2 |= db2_speed_steps_128; break;
    }
  }

  inline Direction direction() const
  {
    return Z21::Utils::getDirection(speedAndDirection);
  }

  inline void setDirection(Direction value)
  {
    Z21::Utils::setDirection(speedAndDirection, value);
  }

  inline bool isEmergencyStop() const
  {
    return Z21::Utils::isEmergencyStop(speedAndDirection, speedSteps());
  }

  inline void setEmergencyStop()
  {
    Z21::Utils::setEmergencyStop(speedAndDirection);
  }

  inline uint8_t speedStep() const
  {
    return Z21::Utils::getSpeedStep(speedAndDirection, speedSteps());
  }

  inline void setSpeedStep(uint8_t value)
  {
    Z21::Utils::setSpeedStep(speedAndDirection, speedSteps(), value);
  }

  bool getFunction(uint8_t index)
  {
    if(index == 0)
      return db4 & flagF0;
    else if(index <= 4)
      return db4 & (1 << (index - 1));
    else if(index <= 12)
      return f5f12 & (1 << (index - 5));
    else if(index <= 20)
      return f13f20 & (1 << (index - 13));
    else if(index <= 28)
      return f21f28 & (1 << (index - 21));
    else
      return false;
  }

  void setFunction(uint8_t index, bool value)
  {
    if(index == 0)
    {
      if(value)
        db4 |= flagF0;
      else
        db4 &= ~flagF0;
    }
    else if(index <= 4)
    {
      const uint8_t flag = (1 << (index - 1));
      if(value)
        db4 |= flag;
      else
        db4 &= ~flag;
    }
    else if(index <= 12)
    {
      const uint8_t flag = (1 << (index - 5));
      if(value)
        f5f12 |= flag;
      else
        f5f12 &= ~flag;
    }
    else if(index <= 20)
    {
      const uint8_t flag = (1 << (index - 13));
      if(value)
        f13f20 |= flag;
      else
        f13f20 &= ~flag;
    }
    else if(index <= 28)
    {
      const uint8_t flag = (1 << (index - 21));
      if(value)
        f21f28 |= flag;
      else
        f21f28 &= ~flag;
    }
  }

  void calcChecksum()
  {
    checksum = xheader;
    for(uint8_t* db = &addressHigh; db < &checksum; db++)
      checksum ^= *db;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanXLocoInfo) == 14);

// Reply to LAN_X_GET_FIRMWARE_VERSION

// Reply to LAN_GET_BROADCASTFLAGS

// Reply to LAN_GET_LOCOMODE
struct LanGetLocoModeReply : Message
{
  uint16_t addressBE;
  LocoMode mode;

  LanGetLocoModeReply(uint16_t _address, LocoMode _mode) :
    Message(sizeof(LanGetLocoModeReply), LAN_GET_LOCO_MODE),
    addressBE{host_to_be(_address)},
    mode{_mode}
  {
  }

  inline uint16_t address() const
  {
    return be_to_host(addressBE);
  }

  inline void setAddress(uint16_t value)
  {
    addressBE = host_to_be(value);
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanGetLocoModeReply) == 7);

// Reply to LAN_GET_TURNOUTMODE

// LAN_RMBUS_DATACHANGED

// LAN_SYSTEMSTATE_DATACHANGED
struct LanSystemStateDataChanged : Message
{
  int16_t mainCurrent= 0; //!< Current on the main track in mA
  int16_t progCurrent = 0; //!< Current on programming track in mA;
  int16_t filteredMainCurrent = 0; //!< Smoothed current on the main track in mA
  int16_t temperature = 0; //!< Command station internal temperature in °C
  uint16_t supplyVoltage = 0; //!< Supply voltage in mV
  uint16_t vccVoltage = 0; //!< Internal voltage, identical to track voltage in mV
  uint8_t centralState = 0; //!< bitmask, see Z21_CENTRALSTATE
  uint8_t centralStateEx = 0; //!< bitmask, see Z21_CENTRALSTATEEX
  uint8_t _reserved1 = 0;
  uint8_t _reserved2 = 0;

  LanSystemStateDataChanged() :
    Message(sizeof(LanSystemStateDataChanged), LAN_SYSTEMSTATE_DATACHANGED)
  {
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanSystemStateDataChanged) == 20);

// LAN_RAILCODATACHANGED

// LAN_LOCONET_Z21_RX

// LAN_LOCONET_Z21_TX

// LAN_LOCONET_FROLAN

// LAN_LOCONET_DISPATCH_ADDR

// LAN_LOCONET_DETECTOR

// LAN_CAN_DETECTOR

//=============================================================================

PRAGMA_PACK_POP

}

inline bool operator ==(const Z21::Message& lhs, const Z21::Message& rhs)
{
  return lhs.dataLen() == rhs.dataLen() && std::memcmp(&lhs, &rhs, lhs.dataLen()) == 0;
}

#endif
