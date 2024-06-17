/**
 * server/src/hardware/protocol/z21/messages.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022,2024 Reinder Feenstra
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
#include "../../../utils/byte.hpp"

class Decoder;

namespace Z21 {

struct Message;

std::string toString(const Message& message, bool raw = false);

enum Header : uint16_t
{
  LAN_GET_SERIAL_NUMBER = 0x10,
  LAN_GET_CODE = 0x18,
  LAN_GET_HWINFO = 0x1A,
  LAN_LOGOFF = 0x30,
  LAN_X = 0x40,
  LAN_SET_BROADCASTFLAGS = 0x50,
  LAN_GET_BROADCASTFLAGS = 0x51,
  LAN_GET_LOCO_MODE = 0x60,
  LAN_SET_LOCO_MODE = 0x61,
  LAN_GET_TURNOUTMODE = 0x70,
  LAN_SET_TURNOUTMODE = 0x71,
  LAN_RMBUS_DATACHANGED = 0x80,
  LAN_RMBUS_GETDATA = 0x81,
  LAN_RMBUS_PROGRAMMODULE = 0x82,
  LAN_SYSTEMSTATE_DATACHANGED = 0x84,
  LAN_SYSTEMSTATE_GETDATA = 0x85,
  LAN_RAILCOM_DATACHANGED = 0x88,
  LAN_RAILCOM_GETDATA = 0x89,
  LAN_LOCONET_Z21_RX = 0xA0,
  LAN_LOCONET_Z21_TX = 0xA1,
  LAN_LOCONET_FROM_LAN = 0xA2,
  LAN_LOCONET_DISPATCH_ADDR = 0xA3,
  LAN_LOCONET_DETECTOR = 0xA4,
  LAN_CAN_DETECTOR = 0xC4,
};

enum class BroadcastFlags : uint32_t
{
  None = 0,

  /// Broadcasts and info messages concerning driving and switching are delivered to the registered clients automatically.
  /// The following messages are concerned:
  /// 2.7 LAN_X_BC_TRACK_POWER_OFF
  /// 2.8 LAN_X_BC_TRACK_POWER_ON
  /// 2.9 LAN_X_BC_PROGRAMMING_MODE
  /// 2.10 LAN_X_BC_TRACK_SHORT_CIRCUIT
  /// 2.14 LAN_X_BC_STOPPED
  /// 4.4 LAN_X_LOCO_INFO (loco address must be subscribed too)
  /// 5.3 LAN_X_TURNOUT_INFO
  PowerLocoTurnoutChanges = 0x00000001,

  /// Changes of the feedback devices on the R-Bus are sent automatically.
  /// Z21 Broadcast messages see 7.1 LAN_RMBUS_DATACHANGED
  RBusChanges = 0x00000002,

  /// Changes of RailCom data of subscribed locomotives are sent automatically.
  /// Z21 Broadcast messages see 8.1 LAN_RAILCOM_DATACHANGED
  RailCOMChanges = 0x00000004,

  /// Changes of the Z21 system status are sent automatically.
  /// Z21 Broadcast messages see 2.18 LAN_SYSTEMSTATE_DATACHANGED
  SystemStatusChanges = 0x00000100,

  /// Extends flag 0x00000001; client now gets LAN_X_LOCO_INFO LAN_X_LOCO_INFO
  /// without having to subscribe to the corresponding locomotive addresses, i.e. for all
  /// controlled locomotives!
  /// Due to the high network traffic, this flag may only be used by adequate PC railroad
  /// automation software and is NOT intended for mobile hand controllers under any
  /// circumstances.
  /// From FW V1.20 bis V1.23: LAN_X_LOCO_INFO is sent for all locomotives.
  /// From FW V1.24: LAN_X_LOCO_INFO is sent for all modified locomotives.
  AllLocoChanges = 0x00010000,

  /// Forwarding messages from LocoNet bus to LAN client without locos and switches.
  LocoNetWithoutLocoAndSwitches = 0x01000000,

  /// Forwarding locomotive-specific LocoNet messages to LAN Client:
  /// OPC_LOCO_SPD, OPC_LOCO_DIRF, OPC_LOCO_SND, OPC_LOCO_F912, OPC_EXP_CMD
  LocoNetLoco = 0x02000000,

  /// Forwarding switch-specific LocoNet messages to LAN client:
  /// OPC_SW_REQ, OPC_SW_REP, OPC_SW_ACK, OPC_SW_STATE
  LocoNetSwitch = 0x04000000,

  /// Sending status changes of LocoNet track occupancy detectors to the LAN client.
  /// See 9.5 LAN_LOCONET_DETECTOR
  LocoNetDetector = 0x08000000,

  ///
  LocoNet = LocoNetWithoutLocoAndSwitches | LocoNetLoco | LocoNetSwitch | LocoNetDetector,

  /// Version 1.29:
  /// Sending changes of RailCom data to the LAN Client.
  /// Client gets LAN_RAILCOM_DATACHANGED without having to subscribe to the
  /// corresponding locomotive addresses, i.e. for all controlled locomotives! Due to the high
  /// network traffic, this flag may only be used by adequate PC railroad automation software
  /// and is NOT intended for mobile hand controllers under any circumstances.
  /// Z21 Broadcast messages see 8.1 LAN_RAILCOM_DATACHANGED
  RailComDataChanged = 0x00040000,

  /// Sending status changes of CAN-Bus track occupancy detectors to the LAN client.
  /// See 10.1 LAN_CAN_DETECTOR
  CANDetector = 0x00080000,
};

enum LocoMode : uint8_t
{
  DCC = 0,
  Motorola = 1,
};

static constexpr uint8_t LAN_X_TURNOUT_INFO = 0x43;
static constexpr uint8_t LAN_X_SET_TURNOUT = 0x53;

static constexpr uint8_t LAN_X_EXT_ACCESSORY_INFO = 0x44;
static constexpr uint8_t LAN_X_SET_EXT_ACCESSORY = 0x54;

static constexpr uint8_t LAN_X_BC = 0x61;

static constexpr uint8_t LAN_X_STATUS_CHANGED = 0x62;
static constexpr uint8_t LAN_X_GET_VERSION_REPLY = 0x63;

//static constexpr uint8_t LAN_X_CV_NACK_SC = 0x12;
//static constexpr uint8_t LAN_X_CV_NACK = 0x13;
//static constexpr uint8_t LAN_X_UNKNOWN_COMMAND = 0x82;

static constexpr uint8_t LAN_X_SET_STOP = 0x80;
static constexpr uint8_t LAN_X_BC_STOPPED = 0x81;

static constexpr uint8_t LAN_X_GET_LOCO_INFO = 0xE3;
static constexpr uint8_t LAN_X_SET_LOCO = 0xE4;
static constexpr uint8_t LAN_X_LOCO_INFO = 0xEF;

static constexpr uint8_t LAN_X_GET_FIRMWARE_VERSION = 0xF1;
static constexpr uint8_t LAN_X_GET_FIRMWARE_VERSION_REPLY = 0xF3;

// db0 for xHeader 0x21
static constexpr uint8_t LAN_X_SET_TRACK_POWER_OFF = 0x80;
static constexpr uint8_t LAN_X_SET_TRACK_POWER_ON = 0x81;

// db0 for xHeader LAN_X_BC
static constexpr uint8_t LAN_X_BC_TRACK_POWER_OFF = 0x00;
static constexpr uint8_t LAN_X_BC_TRACK_POWER_ON = 0x01;
//static constexpr uint8_t LAN_X_BC_PROGRAMMING_MODE = 0x02;
static constexpr uint8_t LAN_X_BC_TRACK_SHORT_CIRCUIT = 0x08;
static constexpr uint8_t LAN_X_UNKNOWN_COMMAND = 0x82;

enum HardwareType : uint32_t
{
  HWT_UNKNOWN = 0,
  HWT_Z21_OLD = 0x00000200, //!< „black Z21” (hardware variant from 2012)
  HWT_Z21_NEW = 0x00000201, //!<  „black Z21”(hardware variant from 2013)
  HWT_SMARTRAIL = 0x00000202, //!< SmartRail (from 2012)
  HWT_Z21_SMALL = 0x00000203, //!< „white z21” starter set variant (from 2013)
  HWT_Z21_START = 0x00000204, //!< „z21 start” starter set variant (from 2016)
  HWT_SINGLE_BOOSTER = 0x00000205, //!< 10806 „Z21 Single Booster” (zLink)
  HWT_DUAL_BOOSTER   = 0x00000206, //!< 10807 „Z21 Dual Booster” (zLink)
  HWT_Z21_XL     = 0x00000211, //!< 10870 „Z21 XL Series” (from 2020)
  HWT_XL_BOOSTER = 0x00000212, //!< 10869 „Z21 XL Booster” (from 2021, zLink)
  HWT_Z21_SWITCH_DECODER = 0x00000301, //!< 10836 „Z21 SwitchDecoder” (zLink)
  HWT_Z21_SIGNAL_DECODER = 0x00000302  //!< 10837 „Z21 SignalDecoder” (zLink)
};

constexpr std::string_view toString(HardwareType value)
{
  switch(value)
  {
    case HWT_Z21_OLD:
      return "Black Z21 (hardware variant from 2012)";

    case HWT_Z21_NEW:
      return "Black Z21 (hardware variant from 2013)";

    case HWT_SMARTRAIL:
      return "SmartRail (from 2012)";

    case HWT_Z21_SMALL:
      return "White Z21 (starter set variant from 2013)";

    case HWT_Z21_START :
      return "Z21 start (starter set variant from 2016)";

    case HWT_SINGLE_BOOSTER :
      return "Z21 Single Booster (10806, zLink)";

    case HWT_DUAL_BOOSTER :
      return "Z21 Dual Booster (10807, zLink)";

    case HWT_Z21_XL :
      return "Z21 XL Series (from 2020)";

    case HWT_XL_BOOSTER :
      return "Z21 XL Booster (from 2021, zLink)";

    case HWT_Z21_SWITCH_DECODER :
      return "Z21 SwitchDecoder (zLink)";

    case HWT_Z21_SIGNAL_DECODER :
      return "Z21 SignalDecoder (zLink)";

    case HWT_UNKNOWN:
      break;
  }

  return {};
}

enum class CommandStationId : uint8_t
{
  Z21 = 0x12,
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

  void updateChecksum(uint8_t len);

  inline void updateChecksum()
  {
    updateChecksum(xheader & 0x0F);
  }

  static bool isChecksumValid(const LanX& lanX);

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
struct LanXGetVersion : LanX
{
  uint8_t db0 = 0x21;
  uint8_t checksum = 0x00;

  LanXGetVersion() :
    LanX(sizeof(LanXGetVersion), 0x21)
  {
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanXGetVersion) == 7);

// LAN_X_GET_FIRMWARE_VERSION
struct LanXGetFirmwareVersion : LanX
{
  uint8_t db0 = 0x0A;
  uint8_t checksum = 0xFB;

  LanXGetFirmwareVersion() :
    LanX(sizeof(LanXGetFirmwareVersion), LAN_X_GET_FIRMWARE_VERSION)
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
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanXGetStatus) == 7);

// LAN_X_SET_TRACK_POWER_OFF
struct LanXSetTrackPowerOff : LanX
{
  uint8_t db0 = LAN_X_SET_TRACK_POWER_OFF;
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
  uint8_t db0 = LAN_X_SET_TRACK_POWER_ON;
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
struct LanXGetTurnoutInfo : LanX
{
  uint8_t addressLSB;
  uint8_t addressMSB;
  uint8_t checksum;

  LanXGetTurnoutInfo(uint16_t address)
    : LanX(sizeof(LanXGetTurnoutInfo), LAN_X_TURNOUT_INFO)
  {
    setAddress(address);
    updateChecksum();
  }

  inline uint16_t rawAddress() const
  {
    return to16(addressLSB, addressMSB);
  }

  inline uint16_t address() const
  {
    return 1 + rawAddress();
  }

  void setAddress(uint16_t value)
  {
    assert(value >= 1 && value <= 2048);
    value--;
    addressMSB = high8(value);
    addressLSB = low8(value);
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanXGetTurnoutInfo) == 8);

// LAN_X_SET_TURNOUT
struct LanXSetTurnout : LanX
{
  static constexpr uint8_t db2Port = 0x01;
  static constexpr uint8_t db2Activate = 0x08;
  static constexpr uint8_t db2Queue = 0x20;

  uint8_t addressMSB;
  uint8_t addressLSB;
  uint8_t db2 = 0x80;
  uint8_t checksum;

  LanXSetTurnout(uint16_t address, bool port, bool activate, bool queue = false)
    : LanX(sizeof(LanXSetTurnout), LAN_X_SET_TURNOUT)
  {
    setAddress(address);

    if(queue)
      db2 |= db2Queue;
    if(activate)
      db2 |= db2Activate;
    if(port)
      db2 |= db2Port;

    updateChecksum();
  }

  inline uint16_t rawAddress() const
  {
    return to16(addressLSB, addressMSB);
  }

  inline uint16_t address() const
  {
    return 1 + rawAddress();
  }

  void setAddress(uint16_t value)
  {
    assert(value >= 1 && value <= 2048);
    value--;
    addressMSB = high8(value);
    addressLSB = low8(value);
  }

  inline bool activate() const
  {
    return db2 & db2Queue;
  }

  inline bool queue() const
  {
    return db2 & db2Queue;
  }

  inline bool port() const
  {
    return db2 & db2Port;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanXSetTurnout) == 9);

// LAN_X_SET_EXT_ACCESSORY
struct LanXSetExtAccessory : LanX
{
  uint8_t addressMSB;
  uint8_t addressLSB;
  uint8_t db2;
  uint8_t db3 = 0x00; // must be 0x00
  uint8_t checksum;

  LanXSetExtAccessory(uint16_t address)
    : LanX(sizeof(LanXSetExtAccessory), LAN_X_SET_EXT_ACCESSORY)
    , addressMSB(high8(address + 3))
    , addressLSB(low8(address + 3))
  {
  }

  LanXSetExtAccessory(uint16_t address, uint8_t aspect)
    : LanXSetExtAccessory(address)
  {
    db2 = aspect;
    updateChecksum();
  }

  LanXSetExtAccessory(uint16_t address, bool dir, uint8_t powerOnTime)
    : LanXSetExtAccessory(address)
  {
    assert(powerOnTime <= 127);
    db2 = powerOnTime & 0x7F;
    if(dir)
    {
      db2 |= 0x80;
    }
    updateChecksum();
  }

  inline uint16_t rawAddress() const
  {
    return to16(addressLSB, addressMSB);
  }

  inline uint16_t address() const
  {
    return rawAddress() + 3;
  }

  inline uint8_t aspect() const
  {
    return db2;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanXSetExtAccessory) == 10);

// LAN_X_GET_EXT_ACCESSORY_INFO
struct LanXGetExtAccessoryInfo : LanX
{
  uint8_t addressMSB;
  uint8_t addressLSB;
  uint8_t db2 = 0x00; // Reserved for future extension, must be 0x00
  uint8_t checksum;

  LanXGetExtAccessoryInfo(uint16_t address)
    : LanX(sizeof(LanXGetExtAccessoryInfo), LAN_X_EXT_ACCESSORY_INFO)
    , addressMSB(high8(address + 3))
    , addressLSB(low8(address + 3))
  {
  }

  inline uint16_t rawAddress() const
  {
    return to16(addressLSB, addressMSB);
  }

  inline uint16_t address() const
  {
    return rawAddress() + 3;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanXGetExtAccessoryInfo) == 9);

// LAN_X_EXT_ACCESSORY_INFO
struct LanXExtAccessoryInfo : LanX
{
  uint8_t addressMSB;
  uint8_t addressLSB;
  uint8_t db2;
  uint8_t db3 = 0x00;
  uint8_t checksum;

  LanXExtAccessoryInfo(uint16_t address)
    : LanX(sizeof(LanXExtAccessoryInfo), LAN_X_EXT_ACCESSORY_INFO)
    , addressMSB(high8(address + 3))
    , addressLSB(low8(address + 3))
  {
  }

  LanXExtAccessoryInfo(uint16_t address, uint8_t aspect, bool isUnknown)
    : LanXExtAccessoryInfo(address)
  {
    db2 = aspect;
    db3 = isUnknown ? 0xFF : 0x00; // 0xFF represent Unknown Data
    updateChecksum();
  }

  LanXExtAccessoryInfo(uint16_t address, bool dir, uint8_t powerOnTime)
    : LanXExtAccessoryInfo(address)
  {
    assert(powerOnTime <= 127);
    db2 = powerOnTime & 0x7F;
    if(dir)
    {
      db2 |= 0x80;
    }
    updateChecksum();
  }

  inline uint16_t rawAddress() const
  {
    return to16(addressLSB, addressMSB);
  }

  inline uint16_t address() const
  {
    return rawAddress() + 3;
  }

  inline bool isDataValid() const
  {
    return db3 == 0x00;
  }

  inline uint8_t aspect() const
  {
    return db2;
  }

  inline bool direction() const
  {
    return db2 & 0x80;
  }

  inline bool powerOnTime() const
  {
    return db2 & 0x7F;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanXExtAccessoryInfo) == 10);

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
    LanX(sizeof(LanXGetLocoInfo), LAN_X_GET_LOCO_INFO)
  {
    setAddress(address, longAddress);
    updateChecksum();
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
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanXGetLocoInfo) == 9);

// LAN_X_SET_LOCO_DRIVE
struct LanXSetLocoDrive : LanX
{
  uint8_t db0;
  uint8_t addressHigh;
  uint8_t addressLow;
  uint8_t speedAndDirection = 0;
  uint8_t checksum;

  LanXSetLocoDrive() :
    LanX(sizeof(LanXSetLocoDrive), LAN_X_SET_LOCO)
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

  inline void setAddress(uint16_t address, bool longAddress)
  {
    addressHigh = longAddress ? (0xC0 | (address >> 8)) : 0x00;
    addressLow = longAddress ? address & 0xFF : address & 0x7F;
  }

  inline void setSpeedSteps(uint8_t steps)
  {
    switch(steps)
    {
    case 14:
      db0 = 0x10;
      break;
    case 28:
      db0 = 0x12;
      break;
    case 126:
    case 128:
    default:
      db0 = 0x13;
      break;
    }
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
    assert(value != Direction::Unknown);
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

  static constexpr uint8_t functionNumberMax = 28;
  static constexpr uint8_t functionNumberMask = 0x3F;
  static constexpr uint8_t switchTypeMask = 0xC0;
  static constexpr uint8_t switchTypeShift = 6;

  uint8_t db0 = 0xf8;
  uint8_t addressHigh;
  uint8_t addressLow;
  uint8_t db3 = 0;
  uint8_t checksum;

  LanXSetLocoFunction() :
    LanX(sizeof(LanXSetLocoFunction), LAN_X_SET_LOCO)
  {
  }

  LanXSetLocoFunction(uint16_t address, bool longAddress, uint8_t functionIndex, SwitchType value)
    : LanXSetLocoFunction()
  {
    setAddress(address, longAddress);
    setFunctionIndex(functionIndex);
    setSwitchType(value);
    updateChecksum();
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

  inline SwitchType switchType() const
  {
    return static_cast<SwitchType>(db3 >> switchTypeShift);
  }

  inline void setSwitchType(SwitchType value)
  {
    db3 = (db3 & functionNumberMask) | (static_cast<uint8_t>(value) << switchTypeShift);
  }

  inline uint8_t functionIndex() const
  {
    return db3 & functionNumberMask;
  }

  inline void setFunctionIndex(uint8_t value)
  {
    assert(value <= functionNumberMax);
    db3 = (db3 & switchTypeMask) | (value & functionNumberMask);
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

  LanSetBroadcastFlags(BroadcastFlags _broadcastFlags = BroadcastFlags::None) :
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
struct LanRMBusGetData : Message
{
  uint8_t groupIndex;

  LanRMBusGetData(uint8_t groupIndex_ = 0)
    : Message(sizeof(LanRMBusGetData), LAN_RMBUS_GETDATA)
    , groupIndex{groupIndex_}
  {
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanRMBusGetData) == 5);

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
struct LanLocoNetDetector : Message
{
  enum class Type : uint8_t
  {
    OccupancyDetector = 0x01,
    TransponderEntersBlock = 0x02,
    TransponderExitsBlock = 0x03,

    LissyLocoAddress = 0x10,
    LissyBlockStatus = 0x11,
    LissySpeed = 0x12,

    StationaryInterrogateRequest = 0x80,
    ReportAddress = 0x81,
    StatusRequestLissy = 0x82,
  };

  Type type;
  uint16_t feedbackAddressLE;

  LanLocoNetDetector(uint16_t dataLen, Type type_, uint16_t feedbackAddress)
    : Message(dataLen, LAN_LOCONET_DETECTOR)
    , type{type_}
    , feedbackAddressLE{host_to_le(feedbackAddress)}
  {
  }

  uint16_t feedbackAddress() const
  {
    return le_to_host(feedbackAddressLE);
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanLocoNetDetector) == 7);

struct LanLocoNetDetectorOccupancyDetector : LanLocoNetDetector
{
  uint8_t occupied;

  LanLocoNetDetectorOccupancyDetector(uint16_t feedbackAddress, bool occupied_ = false)
    : LanLocoNetDetector(sizeof(LanLocoNetDetectorOccupancyDetector), Type::OccupancyDetector, feedbackAddress)
    , occupied(occupied_ ? 1 : 0)
  {
  }

  bool isOccupied() const
  {
    return occupied != 0;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanLocoNetDetectorOccupancyDetector) == 8);

struct LanLocoNetDetectorTransponderEntersExitsBlock : LanLocoNetDetector
{
  uint8_t transponderAddressLow;
  uint8_t transponderAddressHigh;

  LanLocoNetDetectorTransponderEntersExitsBlock(Type type_, uint16_t feedbackAddress, uint16_t transponderAddress)
    : LanLocoNetDetector(sizeof(LanLocoNetDetectorTransponderEntersExitsBlock), type_, feedbackAddress)
    , transponderAddressLow(transponderAddress >> 8)
    , transponderAddressHigh(transponderAddress & 0xFF)
  {
    assert(type == Type::TransponderEntersBlock || type == Type::TransponderExitsBlock);
  }

  uint16_t transponderAddress() const
  {
    return (static_cast<uint16_t>(transponderAddressHigh) << 8) | transponderAddressLow;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanLocoNetDetectorTransponderEntersExitsBlock) == 9);

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

// Reply to LAN_X_GET_VERSION
struct LanXGetVersionReply : LanX
{
  uint8_t db0 = 0x21;
  uint8_t xBusVersionBCD;
  CommandStationId commandStationId;
  uint8_t checksum;

  LanXGetVersionReply()
    : LanX(sizeof(LanXGetVersionReply), LAN_X_GET_VERSION_REPLY)
  {
  }

  LanXGetVersionReply(uint8_t _xBusVersion, CommandStationId _commandStationId)
    : LanXGetVersionReply()
  {
    setXBusVersion(_xBusVersion);
    commandStationId = _commandStationId;
    updateChecksum();
  }

  inline uint8_t xBusVersion() const
  {
    return Utils::fromBCD(xBusVersionBCD);
  }

  inline void setXBusVersion(uint8_t value)
  {
    assert(value < 100);
    xBusVersionBCD = Utils::toBCD(value);
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanXGetVersionReply) == 9);

// Reply to LAN_GET_CODE

// LAN_X_GET_FIRMWARE_VERSION
struct LanXGetFirmwareVersionReply : LanX
{
  uint8_t db0 = 0x0A;
  uint8_t majorBCD;
  uint8_t minorBCD;
  uint8_t checksum;

  LanXGetFirmwareVersionReply() :
    LanX(sizeof(LanXGetFirmwareVersionReply), LAN_X_GET_FIRMWARE_VERSION_REPLY)
  {
  }

  LanXGetFirmwareVersionReply(uint8_t _major, uint8_t _minor) :
    LanXGetFirmwareVersionReply()
  {
    setVersionMajor(_major);
    setVersionMinor(_minor);
    updateChecksum();
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
} ATTRIBUTE_PACKED;
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
    return Utils::fromBCD((le_to_host(firmwareVersionLE) >> 8) & 0xFF);
  }

  uint8_t firmwareVersionMinor() const
  {
    return Utils::fromBCD(le_to_host(firmwareVersionLE) & 0xFF);
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanGetHardwareInfoReply) == 12);

// LAN_X_TURNOUT_INFO
struct LanXTurnoutInfo : LanX
{
  uint8_t addressMSB;
  uint8_t addressLSB;
  uint8_t db2;
  uint8_t checksum;

  LanXTurnoutInfo(uint16_t address, bool port, bool isUnknown)
    : LanX(sizeof(LanXTurnoutInfo), LAN_X_TURNOUT_INFO)
  {
    setAddress(address);

    if(isUnknown)
      db2 = 0;
    else
      db2 = port ? 0x02 : 0x01;

    updateChecksum();
  }

  inline uint16_t rawAddress() const
  {
    return to16(addressLSB, addressMSB);
  }

  inline uint16_t address() const
  {
    return 1 + rawAddress();
  }

  inline bool positionUnknown() const
  {
    return db2 == 0;
  }

  inline bool state() const
  {
    return db2 == 0x02;
  }

  void setAddress(uint16_t value)
  {
    assert(value >= 1 && value <= 2048);
    value--;
    addressMSB = high8(value);
    addressLSB = low8(value);
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanXTurnoutInfo) == 9);

// LAN_X_BC_TRACK_POWER_OFF
struct LanXBCTrackPowerOff : LanX
{
  uint8_t db0 = LAN_X_BC_TRACK_POWER_OFF;
  uint8_t checksum = 0x61;

  LanXBCTrackPowerOff() :
    LanX(sizeof(LanXBCTrackPowerOff), LAN_X_BC)
  {
  }
} ATTRIBUTE_PACKED;
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
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanXBCTrackPowerOn) == 7);

// LAN_X_BC_PROGRAMMING_MODE

// LAN_X_BC_TRACK_SHORT_CIRCUIT
struct LanXBCTrackShortCircuit : LanX
{
  uint8_t db0 = LAN_X_BC_TRACK_SHORT_CIRCUIT;
  uint8_t checksum = 0x69;

  LanXBCTrackShortCircuit() :
      LanX(sizeof(LanXBCTrackShortCircuit), LAN_X_BC)
  {
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanXBCTrackShortCircuit) == 7);

// LAN_X_CV_NACK_SC

// LAN_X_CV_NACK

// LAN_X_UNKNOWN_COMMAND
struct LanXUnknownCommand : LanX
{
  uint8_t db0 = LAN_X_UNKNOWN_COMMAND;
  uint8_t checksum = xheader ^ db0;

  LanXUnknownCommand() :
      LanX(sizeof(LanXUnknownCommand), LAN_X_BC)
  {
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanXUnknownCommand) == 7);

// LAN_X_STATUS_CHANGED
struct LanXStatusChanged : LanX
{
  uint8_t db0 = 0x22;
  uint8_t db1 = 0;
  uint8_t checksum;

  LanXStatusChanged() :
    LanX(sizeof(LanXStatusChanged), LAN_X_STATUS_CHANGED)
  {
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanXStatusChanged) == 8);

// Reply to LAN_X_GET_VERSION

// LAN_X_CV_RESULT

// LAN_X_BC_STOPPED
struct LanXBCStopped : LanX
{
  uint8_t db0 = 0x00;
  uint8_t checksum = 0x81;

  LanXBCStopped() :
    LanX(sizeof(LanXBCStopped), LAN_X_BC_STOPPED)
  {
  }
} ATTRIBUTE_PACKED;
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
  static constexpr uint8_t supportedFunctionIndexMax = 31; ///< \sa functionIndexMax

  static constexpr uint8_t minMessageSize = 7 + 7;
  static constexpr uint8_t maxMessageSize = 7 + 14;

  uint8_t addressHigh = 0; //db0
  uint8_t addressLow = 0;  //db1
  uint8_t db2 = 0;
  uint8_t speedAndDirection = 0; //db3
  uint8_t db4 = 0;
  uint8_t f5f12 = 0;  //db5
  uint8_t f13f20 = 0; //db6
  uint8_t f21f28 = 0; //db7
  uint8_t db8 = 0;    //db8 is f29f31 (firmware >= 1.42) otherwise checksum
  uint8_t db9 = 0;    //checksum (firmware >= 1.42)

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
      case 126:
      case 128:
      default:  db2 |= db2_speed_steps_128; break;
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

  inline bool supportsF29F31() const
  {
    //Firmware >= 1.42 adds db8 to store F29-F31 so dataLen increases to 15
    return dataLen() >= 15;
  }

  /*!
   * \brief Get maximum fuction index stored in this message
   * \return Maximum index
   *
   * \note There is also a function at index 0 so count it
   *
   * Maximum function index depends on Z21 firmware protocol version
   * versions <  1.42 support up to F28
   * versions >= 1.42 support up to F31
   *
   * Messages are backward compatible but older version will only read
   * up to their maximum function number
   *
   * We currently support up to F31 in trasmission and reception \sa supportedFunctionIndexMax
   */
  inline uint8_t functionIndexMax() const
  {
    if(supportsF29F31())
      return 31;
    return 28;
  }

  bool getFunction(uint8_t index) const
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
    else if(index <= 31 && supportsF29F31())
      return db8 & (1 << (index - 29));
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
    else if(index <= 31 && supportsF29F31())
    {
      const uint8_t flag = (1 << (index - 29));
      if(value)
        db8 |= flag;
      else
        db8 &= ~flag;
    }
  }

  inline void updateChecksum()
  {
    //Data length - 7 Z21 header bytes + 1 byte for db0
    LanX::updateChecksum(dataLen() - 6);
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanXLocoInfo) >= LanXLocoInfo::minMessageSize &&
              sizeof(LanXLocoInfo) <= LanXLocoInfo::maxMessageSize);

// Reply to LAN_X_GET_FIRMWARE_VERSION

// Reply to LAN_GET_BROADCASTFLAGS
struct LanGetBroadcastFlagsReply : Message
{
  BroadcastFlags broadcastFlagsLE; // LE

  LanGetBroadcastFlagsReply(BroadcastFlags _broadcastFlags = BroadcastFlags::None) :
      Message(sizeof(LanGetBroadcastFlagsReply), LAN_GET_BROADCASTFLAGS),
      broadcastFlagsLE{host_to_le(_broadcastFlags)}
  {
  }

  inline BroadcastFlags broadcastFlags() const
  {
    return le_to_host(broadcastFlagsLE);
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanGetBroadcastFlagsReply) == 8);

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
struct LanRMBusDataChanged : Message
{
  uint8_t groupIndex;
  uint8_t feedbackStatus[10];

  static constexpr uint8_t feedbackStatusCount = 8 * sizeof(feedbackStatus);

  LanRMBusDataChanged(uint8_t groupIndex_ = 0)
    : Message(sizeof(LanRMBusDataChanged), LAN_RMBUS_DATACHANGED)
    , groupIndex{groupIndex_}
  {
  }

  inline bool getFeedbackStatus(uint8_t index) const
  {
    assert(index < feedbackStatusCount);
    return feedbackStatus[index >> 3] & (1 << (index & 0x7));
  }

  inline void setFeedbackStatus(uint8_t index, bool value)
  {
    assert(index < feedbackStatusCount);
    if(value)
      feedbackStatus[index >> 3] |= (1 << (index & 0x7));
    else
      feedbackStatus[index >> 3] &= ~static_cast<uint8_t>(1 << (index & 0x7));
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanRMBusDataChanged) == 15);

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

constexpr std::string_view toString(LanXSetLocoFunction::SwitchType value)
{
  switch(value)
  {
    case LanXSetLocoFunction::SwitchType::Off:
      return "off";
    case LanXSetLocoFunction::SwitchType::On:
      return "on";
    case LanXSetLocoFunction::SwitchType::Toggle:
      return "toggle";
    case LanXSetLocoFunction::SwitchType::Invalid:
      return "invalid";
  }
  return {};
}

}

inline bool operator ==(const Z21::Message& lhs, const Z21::Message& rhs)
{
  return lhs.dataLen() == rhs.dataLen() && std::memcmp(&lhs, &rhs, lhs.dataLen()) == 0;
}

constexpr Z21::BroadcastFlags operator |(Z21::BroadcastFlags lhs, Z21::BroadcastFlags rhs)
{
  return static_cast<Z21::BroadcastFlags>(
    static_cast<std::underlying_type_t<Z21::BroadcastFlags>>(lhs) |
    static_cast<std::underlying_type_t<Z21::BroadcastFlags>>(rhs));
}

constexpr Z21::BroadcastFlags operator &(Z21::BroadcastFlags lhs, Z21::BroadcastFlags rhs)
{
  return static_cast<Z21::BroadcastFlags>(
    static_cast<std::underlying_type_t<Z21::BroadcastFlags>>(lhs) &
    static_cast<std::underlying_type_t<Z21::BroadcastFlags>>(rhs));
}

#endif
