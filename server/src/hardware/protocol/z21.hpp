/**
 * server/src/hardware/commandstation/protocol/z21.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_Z21_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_Z21_HPP

#include <cstdint>
#include <cstring>
#include <cassert>
#include <traintastic/enum/direction.hpp>
#include "../../utils/endian.hpp"
#include "../../utils/packed.hpp"

namespace Hardware {
  class Decoder;
}

namespace Protocol::Z21 {

namespace Utils {

inline constexpr uint8_t directionFlag = 0x80;

constexpr Direction getDirection(uint8_t db)
{
  return (db & directionFlag) ? Direction::Forward : Direction::Reverse;
}

constexpr void setDirection(uint8_t& db, Direction direction)
{
  if(direction == Direction::Forward)
    db |= directionFlag;
  else
    db &= ~directionFlag;
}

constexpr bool isEmergencyStop(uint8_t db, uint8_t speedSteps)
{
  switch(speedSteps)
  {
    case 126:
      return (db & 0x7F) == 0x01;

    case 28:
      return (db & 0x1F) == 0x01 || (db & 0x1F) == 0x11;

    case 14:
      return (db & 0x0F) == 0x01;
  }
  return true;
}

constexpr void setEmergencyStop(uint8_t& db)
{
  db = (db & directionFlag) | 0x01;
}

constexpr uint8_t getSpeedStep(uint8_t db, uint8_t speedSteps)
{
  switch(speedSteps)
  {
    case 126:
      db &= 0x7F;
      break;

    case 28:
      db = ((db & 0x0F) << 1) | ((db & 0x10) >> 4);
      break;

    case 14:
      db &= 0x0F;
      break;

    default:
      return 0;
  }
  return db > 1 ? db - 1 : 0; // step 1 = EStop
}

constexpr void setSpeedStep(uint8_t& db, uint8_t speedSteps, uint8_t speedStep)
{
  db &= directionFlag; // preserve direction flag
  if(++speedStep > 1)
    switch(speedSteps)
    {
      case 126:
        db |= speedStep & 0x7F;
        break;

      case 28:
        db |= ((speedStep >> 1) & 0x0F) | ((speedStep << 4) & 0x01);
        break;

      case 14:
        db |= speedStep & 0x0F;
        break;
    }
}

constexpr uint8_t toBCD(uint8_t value)
{
  return ((value / 10) << 4) | (value % 10);
}

constexpr uint8_t fromBCD(uint8_t value)
{
  return ((value >> 4) * 10) + (value & 0x0F);
}

}

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

PRAGMA_PACK_PUSH_1

class Message
{
  protected:
    uint16_t m_dataLen; //!< DataLen (little endian): Total length over the entire data set including DataLen, Header and Data, i.e. DataLen = 2+2+n.
    uint16_t m_header;  //!< Header (little endian): Describes the Command and the Protocol’s group. \see Header

  public:
    Message(uint16_t _dataLen, Header _header) :
      m_dataLen{host_to_le(_dataLen)},
      m_header{host_to_le(_header)}
    {
    }

    inline uint16_t dataLen() const
    {
      return le_to_host(m_dataLen);
    }

    inline Header header() const
    {
      return static_cast<Header>(le_to_host(m_header));
    }
} ATTRIBUTE_PACKED;
static_assert(sizeof(Message) == 4);

class LanGetSerialNumber : public Message
{
  public:
    LanGetSerialNumber() :
      Message(sizeof(LanGetSerialNumber), LAN_GET_SERIAL_NUMBER)
    {
    }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanGetSerialNumber) == 4);

class LanGetSerialNumberReply : public Message
{
  protected:
    uint32_t m_serialNumber; // LE

  public:
    LanGetSerialNumberReply(uint32_t _serialNumber) :
      Message(sizeof(LanGetSerialNumberReply), LAN_GET_SERIAL_NUMBER),
      m_serialNumber{host_to_le(_serialNumber)}
    {
    }

    inline uint32_t serialNumber() const
    {
      return le_to_host(m_serialNumber);
    }

    inline void setSerialNumber(uint32_t value)
    {
      m_serialNumber = host_to_le(value);
    }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanGetSerialNumberReply) == 8);





// hw_info

class LanLogoff : public Message
{
  LanLogoff() :
    Message(sizeof(LanLogoff), LAN_LOGOFF)
  {
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanLogoff) == 4);



class LanGetLocoMode : public Message
{
  protected:
    uint16_t m_address; // BE

  public:
    LanGetLocoMode(uint16_t _address = 0) :
      Message(sizeof(LanGetLocoMode), LAN_GET_LOCO_MODE)
    {
      setAddress(_address);
    }

    inline uint16_t address() const
    {
      return be_to_host(m_address);
    }

    inline void setAddress(uint16_t value)
    {
      m_address = host_to_be(value);
    }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanGetLocoMode) == 6);

class LanGetLocoModeReply : public Message
{
  protected:
    uint16_t m_address; // BE
    LocoMode m_mode;

  public:
    LanGetLocoModeReply(uint16_t _address = 0, LocoMode _mode = DCC) :
      Message(sizeof(LanGetLocoModeReply), LAN_GET_LOCO_MODE),
      m_address{host_to_be(_address)}
    {
    }

    inline uint16_t address() const
    {
      return be_to_host(m_address);
    }

    inline void setAddress(uint16_t value)
    {
      m_address = host_to_be(value);
    }

    inline LocoMode locoMode() const
    {
      return m_mode;
    }

    inline void setLocoMode(LocoMode value)
    {
      m_mode = value;
    }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanGetLocoModeReply) == 7);

class LanSetLocoMode : public Message
{
  protected:
    uint16_t m_address; // BE
    LocoMode m_mode;

  public:
    LanSetLocoMode(uint16_t _address = 0, LocoMode _mode = DCC) :
      Message(sizeof(LanSetLocoMode), LAN_SET_LOCO_MODE),
      m_address{host_to_be(_address)}
    {
    }

    inline uint16_t address() const
    {
      return be_to_host(m_address);
    }

    inline void setAddress(uint16_t value)
    {
      m_address = host_to_be(value);
    }

    inline LocoMode locoMode() const
    {
      return m_mode;
    }

    inline void setLocoMode(LocoMode value)
    {
      m_mode = value;
    }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanSetLocoMode) == 7);

PRAGMA_PACK_POP













}


#define Z21_LAN_GET_SERIAL_NUMBER 0x10
#define Z21_LAN_GET_HWINFO 0x1A
#define Z21_LAN_LOGOFF 0x30
#define Z21_LAN_X 0x40
  #define Z21_LAN_X_SET_STOP 0x80
  #define Z21_LAN_X_TURNOUT_INFO 0x43
  #define Z21_LAN_X_BC 0x61
    #define Z21_LAN_X_BC_TRACK_POWER_OFF 0x00
    #define Z21_LAN_X_BC_TRACK_POWER_ON 0x01
    #define Z21_LAN_X_BC_PROGRAMMING_MODE 0x02
    #define Z21_LAN_X_BC_TRACK_SHORT_CIRCUIT 0x08
    #define Z21_LAN_X_CV_NACK_SC 0x12
    #define Z21_LAN_X_CV_NACK 0x13
    #define Z21_LAN_X_UNKNOWN_COMMAND 0x82
  #define Z21_LAN_X_BC_STOPPED 0x81
  #define Z21_LAN_X_LOCO_INFO 0xEF
#define Z21_LAN_SET_BROADCASTFLAGS 0x50
#define Z21_LAN_GET_BROADCASTFLAGS 0x51
#define Z21_LAN_SYSTEMSTATE_DATACHANGED 0x84
#define Z21_LAN_SYSTEMSTATE_GETDATA 0x85
#define Z21_LAN_LOCONET_Z21_RX 0xA0
#define Z21_LAN_LOCONET_Z21_TX 0xA1

#define Z21_HWT_Z21_OLD 0x00000200 //!< „black Z21” (hardware variant from 2012)
#define Z21_HWT_Z21_NEW 0x00000201 //!<  „black Z21”(hardware variant from 2013)
#define Z21_HWT_SMARTRAIL 0x00000202 //!< SmartRail (from 2012)
#define Z21_HWT_Z21_SMALL 0x00000203 //!< „white z21” starter set variant (from 2013)
#define Z21_HWT_Z21_START 0x00000204 //!< „z21 start” starter set variant (from 2016)

#define Z21_CENTRALSTATE_EMERGENCYSTOP 0x01 //!< The emergency stop is switched on
#define Z21_CENTRALSTATE_TRACKVOLTAGEOFF 0x02 //!< The track voltage is switched off
#define Z21_CENTRALSTATE_SHORTCIRCUIT 0x04 //!< Short circuit
#define Z21_CENTRALSTATE_PROGRAMMINGMODEACTIVE 0x20 //!< The programming mode is active

#define Z21_CENTRALSTATEEX_HIGHTEMPERATURE 0x01 //!< Temperature too high
#define Z21_CENTRALSTATEEX_POWERLOST 0x02 //!< Input voltage too low
#define Z21_CENTRALSTATEEX_SHORTCIRCUITEXTERNAL 0x04 //!< Short circuit at the external booster output
#define Z21_CENTRALSTATEEX_SHORTCIRCUITINTERNAL 0x08 //!< Short circuit at the main track or programming track

PRAGMA_PACK_PUSH_1

struct z21_lan_header
{
  uint16_t dataLen; // LE
  uint16_t header;  // LE
} ATTRIBUTE_PACKED;
static_assert(sizeof(z21_lan_header) == 0x04);


struct z21_lan_get_hwinfo : z21_lan_header
{
  z21_lan_get_hwinfo()
  {
    dataLen = sizeof(z21_lan_get_hwinfo);
    header = Z21_LAN_GET_HWINFO;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(z21_lan_get_hwinfo) == 0x04);

struct z21_lan_get_hwinfo_reply : z21_lan_header
{
  uint32_t hardwareType; // LE
  uint32_t firmwareVersion; // LE

  z21_lan_get_hwinfo_reply(uint32_t _hardwareType = 0, uint8_t _firmwareVersionMajor = 0, uint8_t _firmwareVersionMinor = 0) :
    hardwareType{host_to_le(_hardwareType)},
    firmwareVersion{host_to_le(static_cast<uint32_t>(Protocol::Z21::Utils::toBCD(_firmwareVersionMajor)) << 8 | Protocol::Z21::Utils::toBCD(_firmwareVersionMinor))}
  {
    dataLen = sizeof(z21_lan_get_hwinfo_reply);
    header = Z21_LAN_GET_HWINFO;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(z21_lan_get_hwinfo_reply) == 0x0C);

struct z21_lan_logoff : z21_lan_header
{
  z21_lan_logoff()
  {
    dataLen = sizeof(z21_lan_logoff);
    header = Z21_LAN_LOGOFF;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(z21_lan_logoff) == 0x04);

struct z21_lan_x : z21_lan_header
{
  uint8_t xheader;

  z21_lan_x()
  {
    header = Z21_LAN_X;
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
static_assert(sizeof(z21_lan_x) == 0x05);




struct z21_lan_x_get_firmware_version : z21_lan_x
{
  uint8_t db0 = 0x0A;
  uint8_t checksum = 0xFB;

  z21_lan_x_get_firmware_version()
  {
    dataLen = sizeof(z21_lan_x_get_firmware_version);
    xheader = 0xF1;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(z21_lan_x_get_firmware_version) == 7);

struct z21_lan_x_get_firmware_version_reply : z21_lan_x
{
  uint8_t db0 = 0x0A;
  uint8_t majorBCD;
  uint8_t minorBCD;
  uint8_t checksum;

  z21_lan_x_get_firmware_version_reply()
  {
    dataLen = sizeof(z21_lan_x_get_firmware_version_reply);
    xheader = 0xF3;
  }

  z21_lan_x_get_firmware_version_reply(uint8_t _major, uint8_t _minor) :
    z21_lan_x_get_firmware_version_reply()
  {
    setVersionMajor(_major);
    setVersionMinor(_minor);
    calcChecksum();
  }

  inline uint8_t versionMajor() const
  {
    return Protocol::Z21::Utils::fromBCD(majorBCD);
  }

  inline uint8_t versionMinor() const
  {
    return Protocol::Z21::Utils::fromBCD(minorBCD);
  }

  inline void setVersionMajor(uint8_t value)
  {
    assert(value < 100);
    majorBCD = Protocol::Z21::Utils::toBCD(value);
  }

  inline void setVersionMinor(uint8_t value)
  {
    assert(value < 100);
    minorBCD = Protocol::Z21::Utils::toBCD(value);
  }
};
static_assert(sizeof(z21_lan_x_get_firmware_version_reply) == 9);








struct z21_lan_x_set_stop : z21_lan_x
{
  uint8_t checksum = 0x80;

  z21_lan_x_set_stop()
  {
    dataLen = sizeof(z21_lan_x_set_stop);
    xheader = Z21_LAN_X_SET_STOP;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(z21_lan_x_set_stop) == 0x06);

struct z21_lan_x_bc_stopped : z21_lan_x
{
  uint8_t db0 = 0x00;
  uint8_t checksum = 0x80;

  z21_lan_x_bc_stopped()
  {
    dataLen = sizeof(z21_lan_x_bc_stopped);
    xheader = Z21_LAN_X_BC_STOPPED;
  }
};
static_assert(sizeof(z21_lan_x_bc_stopped) == 0x07);

struct z21_lan_x_bc_track_power_off : z21_lan_x
{
  uint8_t db0 = 0x00;
  uint8_t checksum = 0x61;

  z21_lan_x_bc_track_power_off()
  {
    dataLen = sizeof(z21_lan_x_bc_track_power_off);
    xheader = 0x61;
  }
};
static_assert(sizeof(z21_lan_x_bc_track_power_off) == 0x07);

struct z21_lan_x_bc_track_power_on : z21_lan_x
{
  uint8_t db0 = 0x01;
  uint8_t checksum = 0x60;

  z21_lan_x_bc_track_power_on()
  {
    dataLen = sizeof(z21_lan_x_bc_track_power_on);
    xheader = 0x61;
  }
};
static_assert(sizeof(z21_lan_x_bc_track_power_on) == 0x07);








struct z21_lan_x_get_status : z21_lan_x
{
  uint8_t db0 = 0x24;
  uint8_t checksum = 0x05;

  z21_lan_x_get_status()
  {
    dataLen = sizeof(z21_lan_x_get_status);
    xheader = 0x21;
  }
};
static_assert(sizeof(z21_lan_x_get_status) == 0x07);

struct z21_lan_x_status_changed : z21_lan_x
{
  uint8_t db0 = 0x22;
  uint8_t db1 = 0;
  uint8_t checksum;

  z21_lan_x_status_changed()
  {
    dataLen = sizeof(z21_lan_x_status_changed);
    xheader = 0x62;
  }
};
static_assert(sizeof(z21_lan_x_status_changed) == 0x08);

struct z21_lan_x_set_track_power_off : z21_lan_x
{
  uint8_t db0 = 0x80;
  uint8_t checksum = 0xa1;

  z21_lan_x_set_track_power_off()
  {
    dataLen = sizeof(z21_lan_x_set_track_power_off);
    header = Z21_LAN_X;
    xheader = 0x21;
  }
} ATTRIBUTE_PACKED;


struct z21_lan_x_set_track_power_on : z21_lan_x
{
  uint8_t db0 = 0x81;
  uint8_t checksum = 0xa0;

  z21_lan_x_set_track_power_on()
  {
    dataLen = sizeof(z21_lan_x_set_track_power_off);
    xheader = 0x21;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(z21_lan_x_set_track_power_off) == 0x07);

struct z21_lan_x_get_loco_info : z21_lan_x
{
  uint8_t db0 = 0xF0;
  uint8_t addressHigh;
  uint8_t addressLow;
  uint8_t checksum;

  z21_lan_x_get_loco_info(uint16_t address, bool longAddress)
  {
    dataLen = sizeof(z21_lan_x_get_loco_info);
    header = Z21_LAN_X;
    xheader = 0xE3;
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

  inline void calcChecksum()
  {
    checksum = xheader ^ db0 ^ addressHigh ^ addressLow;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(z21_lan_x_get_loco_info) == 0x09);

struct z21_lan_x_loco_info : z21_lan_x
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

  z21_lan_x_loco_info()
  {
    dataLen = sizeof(z21_lan_x_loco_info);
    xheader = 0xEF;
  }

  z21_lan_x_loco_info(const Hardware::Decoder& decoder);

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
    return Protocol::Z21::Utils::getDirection(speedAndDirection);
  }

  inline void setDirection(Direction value)
  {
    Protocol::Z21::Utils::setDirection(speedAndDirection, value);
  }

  inline bool isEmergencyStop() const
  {
    return Protocol::Z21::Utils::isEmergencyStop(speedAndDirection, speedSteps());
  }

  inline void setEmergencyStop()
  {
    Protocol::Z21::Utils::setEmergencyStop(speedAndDirection);
  }

  inline uint8_t speedStep() const
  {
    return Protocol::Z21::Utils::getSpeedStep(speedAndDirection, speedSteps());
  }

  inline void setSpeedStep(uint8_t value)
  {
    Protocol::Z21::Utils::setSpeedStep(speedAndDirection, speedSteps(), value);
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

struct z21_lan_x_set_loco_drive : z21_lan_header
{
  static constexpr uint8_t directionFlag = 0x80;

  uint8_t xheader = 0xe4;
  uint8_t db0;
  uint8_t addressHigh;
  uint8_t addressLow;
  uint8_t speedAndDirection = 0;
  uint8_t checksum;

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
    return Protocol::Z21::Utils::getDirection(speedAndDirection);
  }

  inline void setDirection(Direction value)
  {
    Protocol::Z21::Utils::setDirection(speedAndDirection, value);
  }

  inline bool isEmergencyStop() const
  {
    return Protocol::Z21::Utils::isEmergencyStop(speedAndDirection, speedSteps());
  }

  inline void setEmergencyStop()
  {
    Protocol::Z21::Utils::setEmergencyStop(speedAndDirection);
  }

  inline uint8_t speedStep() const
  {
    return Protocol::Z21::Utils::getSpeedStep(speedAndDirection, speedSteps());
  }

  inline void setSpeedStep(uint8_t value)
  {
    Protocol::Z21::Utils::setSpeedStep(speedAndDirection, speedSteps(), value);
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(z21_lan_x_set_loco_drive) == 0x0a);

struct z21_lan_x_set_loco_function : z21_lan_header
{
  enum class SwitchType
  {
    Off = 0,
    On = 1,
    Toggle = 2,
    Invalid = 3,
  };

  uint8_t xheader = 0xe4;
  uint8_t db0 = 0xf8;
  uint8_t addressHigh;
  uint8_t addressLow;
  uint8_t db3;
  uint8_t checksum;

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
static_assert(sizeof(z21_lan_x_set_loco_function) == 0x0a);

struct z21_lan_systemstate_getdata : z21_lan_header
{
  z21_lan_systemstate_getdata()
  {
    dataLen = sizeof(z21_lan_systemstate_getdata);
    header = Z21_LAN_SYSTEMSTATE_GETDATA;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(z21_lan_systemstate_getdata) == 0x04);

struct z21_lan_set_broadcastflags : z21_lan_header
{
  Protocol::Z21::BroadcastFlags broadcastFlags; // LE

  z21_lan_set_broadcastflags(uint32_t _broadcastFlags = 0) :
    broadcastFlags{_broadcastFlags}
  {
    dataLen = sizeof(z21_lan_set_broadcastflags);
    header = Z21_LAN_SET_BROADCASTFLAGS;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(z21_lan_set_broadcastflags) == 0x08);

struct z21_lan_get_broadcastflags : z21_lan_header
{
  z21_lan_get_broadcastflags()
  {
    dataLen = sizeof(z21_lan_get_broadcastflags);
    header = Z21_LAN_GET_BROADCASTFLAGS;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(z21_lan_get_broadcastflags) == 0x04);

struct z21_lan_systemstate_datachanged : z21_lan_header
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

  z21_lan_systemstate_datachanged()
  {
    dataLen = sizeof(z21_lan_systemstate_datachanged);
    header = Z21_LAN_SYSTEMSTATE_DATACHANGED;
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(z21_lan_systemstate_datachanged) == 0x14);

PRAGMA_PACK_POP


inline bool operator ==(const z21_lan_header& lhs, const z21_lan_header& rhs)
{
  return lhs.dataLen == rhs.dataLen && std::memcmp(&lhs.header, &rhs.header, lhs.dataLen - sizeof(lhs.dataLen)) == 0;
}





inline bool operator ==(const Protocol::Z21::Message& lhs, const Protocol::Z21::Message& rhs)
{
  return lhs.dataLen() == rhs.dataLen() && std::memcmp(&lhs, &rhs, lhs.dataLen()) == 0;
}

#endif
