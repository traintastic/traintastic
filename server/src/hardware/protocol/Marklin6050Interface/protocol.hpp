/**
 * server/src/hardware/protocol/Marklin6050Interface/protocol.hpp
 *
 * Protocol constants for the Märklin 6050/6023 serial interface.
 *
 * Binary protocol (6050): All commands are 2 bytes: command byte + address byte.
 * ASCII protocol (6023/6223): Commands are ASCII strings terminated with CR.
 * Extension protocol: Poll with 255 255, receive change buffer.
 *
 * Copyright (C) 2025
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLIN6050_PROTOCOL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLIN6050_PROTOCOL_HPP

#include <cstdint>

namespace Marklin6050 {

enum class ProtocolMode : uint8_t
{
    Binary,  // 6050 binary protocol
    ASCII    // 6023/6223 ASCII protocol
};

// ============================================================
// Binary protocol (6050)
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

// --- Loco functions F1-F4 ---
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
// ASCII protocol (6023/6223)
// ============================================================

constexpr char AsciiCR = '\r';

// ============================================================
// Extension protocol (custom change buffer)
// ============================================================
//
// Poll:     255 255
// Response: <count> [<event>...]
//
// count = 0: no changes (1 byte response)
// count > 0: that many event packets follow
//
// Event format: <type> [<data>...]
//
// Type 0x01: Global state changed (1 data byte)
//   data byte: bit 0 = power (1=on, 0=off)
//              bit 1 = running (1=go, 0=stop)
//   Total: 2 bytes
//
// Type 0x02: Turnout changed (2 data bytes)
//   data byte 1: address (1-255, 0 = address 256)
//   data byte 2: position (0=red/diverging, 1=green/straight)
//   Total: 3 bytes
//
// Type 0x03: Loco state changed (2 data bytes)
//   data byte 1: address (1-255)
//   data byte 2: bit 0-3 = speed (0-14)
//                bit 4   = F0 (1=on)
//                bit 5   = direction (0=forward, 1=reverse)
//   Total: 3 bytes
//
// Type 0x04: Loco functions F1-F4 changed (2 data bytes)
//   data byte 1: address (1-255)
//   data byte 2: bit 0 = F1
//                bit 1 = F2
//                bit 2 = F3
//                bit 3 = F4
//   Total: 3 bytes
//
// Example: 2 events — global go + loco 5 at speed 3 forward with F0
//   Response: 0x02  0x01 0x03  0x03 0x05 0x13
//             count  |         |
//                    global    loco: addr=5, speed=3|F0=1|dir=0 = 0x13
//
// At 2400 baud, typical response with 3 events: ~10 bytes = ~42ms

namespace Extension {

constexpr uint8_t PollByte        = 255;  // send 255 255 to poll

constexpr uint8_t EventGlobal     = 0x01;
constexpr uint8_t EventTurnout    = 0x02;
constexpr uint8_t EventLocoState  = 0x03;
constexpr uint8_t EventLocoFunc   = 0x04;

// Global state bits
constexpr uint8_t GlobalPowerBit  = 0x01;
constexpr uint8_t GlobalRunBit    = 0x02;

// Loco state data byte encoding
constexpr uint8_t LocoSpeedBits   = 0x0F;  // bits 0-3
constexpr uint8_t LocoF0Bit_Ext   = 0x10;  // bit 4
constexpr uint8_t LocoDirBit      = 0x20;  // bit 5 (0=fwd, 1=rev)

// Loco function data byte encoding
constexpr uint8_t LocoF1Bit       = 0x01;
constexpr uint8_t LocoF2Bit       = 0x02;
constexpr uint8_t LocoF3Bit       = 0x04;
constexpr uint8_t LocoF4Bit       = 0x08;

} // namespace Extension

} // namespace Marklin6050

#endif
