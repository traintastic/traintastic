/**
 * server/src/hardware/commandstation/protocol/z21.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019 Reinder Feenstra
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

#ifndef SERVER_HARDWARE_COMMANDSTATION_PROTOCOL_Z21_HPP
#define SERVER_HARDWARE_COMMANDSTATION_PROTOCOL_Z21_HPP

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
#define Z21_LAN_SYSTEMSTATE_DATACHANGED 0x84
#define Z21_LAN_SYSTEMSTATE_GETDATA 0x85

#define Z21_HWT_Z21_OLD 0x00000200 //!< „black Z21” (hardware variant from 2012)
#define Z21_HWT_Z21_NEW 0x00000201 //!<  „black Z21”(hardware variant from 2013)
#define Z21_HWT_SMARTRAIL 0x00000202 //!< SmartRail (from 2012)
#define Z21_HWT_z21_SMALL 0x00000203 //!< „white z21” starter set variant (from 2013)
#define Z21_HWT_z21_START 0x00000204 //!< „z21 start” starter set variant (from 2016)

#define Z21_CENTRALSTATE_EMERGENCYSTOP 0x01 //!< The emergency stop is switched on
#define Z21_CENTRALSTATE_TRACKVOLTAGEOFF 0x02 //!< The track voltage is switched off
#define Z21_CENTRALSTATE_SHORTCIRCUIT 0x04 //!< Short circuit
#define Z21_CENTRALSTATE_PROGRAMMINGMODEACTIVE 0x20 //!< The programming mode is active

#define Z21_CENTRALSTATEEX_HIGHTEMPERATURE 0x01 //!< Temperature too high
#define Z21_CENTRALSTATEEX_POWERLOST 0x02 //!< Input voltage too low
#define Z21_CENTRALSTATEEX_SHORTCIRCUITEXTERNAL 0x04 //!< Short circuit at the external booster output
#define Z21_CENTRALSTATEEX_SHORTCIRCUITINTERNAL 0x08 //!< Short circuit at the main track or programming track

struct z21_lan_header
{
  uint16_t dataLen; // LE
  uint16_t header;  // LE
} __attribute__((packed));
static_assert(sizeof(z21_lan_header) == 0x04);

struct z21_lan_get_serial_number : z21_lan_header
{
  z21_lan_get_serial_number()
  {
    dataLen = sizeof(z21_lan_get_serial_number);
    header = Z21_LAN_GET_SERIAL_NUMBER;
  }
} __attribute__((packed));
static_assert(sizeof(z21_lan_get_serial_number) == 0x04);

struct z21_lan_get_serial_number_reply : z21_lan_get_serial_number
{
  uint32_t serialNumber; // LE
} __attribute__((packed));
static_assert(sizeof(z21_lan_get_serial_number_reply) == 0x08);

struct z21_lan_get_hwinfo : z21_lan_header
{
  z21_lan_get_hwinfo()
  {
    dataLen = sizeof(z21_lan_get_hwinfo);
    header = Z21_LAN_GET_HWINFO;
  }
} __attribute__((packed));
static_assert(sizeof(z21_lan_get_hwinfo) == 0x04);

struct z21_lan_get_hwinfo_reply : z21_lan_get_hwinfo
{
  uint32_t hardwareType; // LE
  uint32_t firmwareVersion; // LE
} __attribute__((packed));
static_assert(sizeof(z21_lan_get_hwinfo_reply) == 0x0C);

struct z21_lan_logoff : z21_lan_header
{
  z21_lan_logoff()
  {
    dataLen = sizeof(z21_lan_logoff);
    header = Z21_LAN_LOGOFF;
  }
} __attribute__((packed));
static_assert(sizeof(z21_lan_logoff) == 0x04);

struct z21_lan_x : z21_lan_header
{
  uint8_t xheader;
} __attribute__((packed));
static_assert(sizeof(z21_lan_x) == 0x05);

struct z21_lan_x_set_stop : z21_lan_x
{
  uint8_t checksum = 0x80;

  z21_lan_x_set_stop()
  {
    dataLen = sizeof(z21_lan_x_set_stop);
    header = Z21_LAN_X;
    xheader = Z21_LAN_X_SET_STOP;
  }
} __attribute__((packed));
static_assert(sizeof(z21_lan_x_set_stop) == 0x06);

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
} __attribute__((packed));

struct z21_lan_x_set_track_power_on : z21_lan_x
{
  uint8_t db0 = 0x81;
  uint8_t checksum = 0xa0;

  z21_lan_x_set_track_power_on()
  {
    dataLen = sizeof(z21_lan_x_set_track_power_off);
    header = Z21_LAN_X;
    xheader = 0x21;
  }
} __attribute__((packed));

struct z21_lan_x_loco_info : z21_lan_x
{
  uint8_t addressHigh;
  uint8_t addressLow;
  uint8_t db2;
  uint8_t speedAndDirection;
  uint8_t db4;
  uint8_t f5f12;
  uint8_t f13f20;
  uint8_t f21f28;
} __attribute__((packed));

struct z21_lan_x_set_loco_drive : z21_lan_header
{
  uint8_t xheader = 0xe4;
  uint8_t db0;
  uint8_t addressHigh;
  uint8_t addressLow;
  uint8_t speedAndDirection = 0;
  uint8_t checksum;
} __attribute__((packed));
static_assert(sizeof(z21_lan_x_set_loco_drive) == 0x0a);

struct z21_lan_x_set_loco_function : z21_lan_header
{
  uint8_t xheader = 0xe4;
  uint8_t db0 = 0xf8;
  uint8_t addressHigh;
  uint8_t addressLow;
  uint8_t db3;
  uint8_t checksum;
} __attribute__((packed));
static_assert(sizeof(z21_lan_x_set_loco_function) == 0x0a);

struct z21_lan_systemstate_getdata : z21_lan_header
{
  z21_lan_systemstate_getdata()
  {
    dataLen = sizeof(z21_lan_systemstate_getdata);
    header = Z21_LAN_SYSTEMSTATE_GETDATA;
  }
} __attribute__((packed));
static_assert(sizeof(z21_lan_systemstate_getdata) == 0x04);

struct z21_lan_systemstate_datachanged : z21_lan_header
{
  int16_t mainCurrent; //!< Current on the main track in mA
  int16_t progCurrent; //!< Current on programming track in mA;
  int16_t filteredMainCurrent; //!< Smoothed current on the main track in mA
  int16_t temperature; //!< Command station internal temperature in °C
  uint16_t SupplyVoltage; //!< Supply voltage in mV
  uint16_t vccVoltage; //!< Internal voltage, identical to track voltage in mV
  uint8_t centralState; //!< bitmask, see Z21_CENTRALSTATE
  uint8_t centralStateEx; //!< bitmask, see Z21_CENTRALSTATEEX
  uint8_t _reserved1;
  uint8_t _reserved2;
} __attribute__((packed));
static_assert(sizeof(z21_lan_systemstate_datachanged) == 0x14);

#endif
