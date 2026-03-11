/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Kamil Kasprzak
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLIN6050_PROTOCOL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLIN6050_PROTOCOL_HPP

#include <cstdint>

namespace Marklin6050 {

// ============================================================
// Binary protocol (6050/6051)
// ============================================================

// --- Loco speed + F0 ---
constexpr uint8_t LocoSpeedMask    = 0x0F;
constexpr uint8_t LocoF0Bit        = 0x10;
constexpr uint8_t LocoStop         = 0;
constexpr uint8_t LocoSpeedMin     = 1;
constexpr uint8_t LocoSpeedMax     = 14;
constexpr uint8_t LocoDirToggle    = 15;

// --- Accessory ---
constexpr uint8_t AccessoryOff     = 32;
constexpr uint8_t AccessoryGreen   = 33;
constexpr uint8_t AccessoryRed     = 34;

// --- Loco functions F1–F4 ---
constexpr uint8_t FunctionBase     = 64;
constexpr uint8_t FunctionF1       = 0x01;
constexpr uint8_t FunctionF2       = 0x02;
constexpr uint8_t FunctionF3       = 0x04;
constexpr uint8_t FunctionF4       = 0x08;

// --- Global commands ---
constexpr uint8_t GlobalGo         = 96;
constexpr uint8_t GlobalStop       = 97;

// --- S88 feedback polling ---
constexpr uint8_t S88Base          = 128;

// ============================================================
// Extension protocol (custom change-event buffer)
//
// Poll:     255 255
// Response: <count> [<event>...]
//
// count = 0 : no changes (1-byte response)
// count > 0 : that many event packets follow
//
// Event format: <type> [<data>...]
//
// Type 0x01: Global state changed  (1 data byte)
//   bit 0 = power (1=on)  bit 1 = running (1=go)
//
// Type 0x02: Turnout changed  (2 data bytes)
//   byte1 = address (1-255, 0 ? 256)
//   byte2 = position (0=red/diverging, 1=green/straight)
//
// Type 0x03: Loco state changed  (2 data bytes)
//   byte1 = address (1-255)
//   byte2: bits 0-3 = speed, bit 4 = F0, bit 5 = dir (0=fwd)
//
// Type 0x04: Loco functions F1-F4 changed  (2 data bytes)
//   byte1 = address (1-255)
//   byte2: bit0=F1  bit1=F2  bit2=F3  bit3=F4
// ============================================================

namespace Extension {

constexpr uint8_t PollByte        = 255;

constexpr uint8_t EventGlobal     = 0x01;
constexpr uint8_t EventTurnout    = 0x02;
constexpr uint8_t EventLocoState  = 0x03;
constexpr uint8_t EventLocoFunc   = 0x04;

// Global state bits
constexpr uint8_t GlobalPowerBit  = 0x01;
constexpr uint8_t GlobalRunBit    = 0x02;

// Loco state data-byte encoding
constexpr uint8_t LocoSpeedBits   = 0x0F;  // bits 0-3
constexpr uint8_t LocoF0Bit_Ext   = 0x10;  // bit 4
constexpr uint8_t LocoDirBit      = 0x20;  // bit 5 (0=fwd, 1=rev)

// Loco function data-byte encoding
constexpr uint8_t LocoF1Bit       = 0x01;
constexpr uint8_t LocoF2Bit       = 0x02;
constexpr uint8_t LocoF3Bit       = 0x04;
constexpr uint8_t LocoF4Bit       = 0x08;

} // namespace Extension

} // namespace Marklin6050

#endif
