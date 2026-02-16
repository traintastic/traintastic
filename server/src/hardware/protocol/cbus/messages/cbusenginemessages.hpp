/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_MESSAGES_CBUSENGINEMESSAGES_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_MESSAGES_CBUSENGINEMESSAGES_HPP

#include <cassert>
#include <span>
#include "cbusmessage.hpp"
#include "../../../../utils/bit.hpp"
#include "../../../../utils/byte.hpp"

namespace CBUS {

constexpr uint8_t engineFunctionMax = 28;

struct EngineMessage : Message
{
  uint8_t session;

protected:
  EngineMessage(OpCode opc, uint8_t session_)
    : Message{opc}
    , session{session_}
  {
  }
};

struct ReleaseEngine : EngineMessage
{
  ReleaseEngine(uint8_t session_)
    : EngineMessage(OpCode::KLOC, session_)
  {
  }
};

struct QueryEngine : EngineMessage
{
  QueryEngine(uint8_t session_)
    : EngineMessage(OpCode::QLOC, session_)
  {
  }
};

struct SessionKeepAlive : EngineMessage
{
  SessionKeepAlive(uint8_t session_)
    : EngineMessage(OpCode::DKEEP, session_)
  {
  }
};

struct RequestEngineSession : Message
{
  uint8_t addrH;
  uint8_t addrL;

  RequestEngineSession(uint16_t address_, bool longAddress)
    : Message(OpCode::RLOC)
  {
    if(longAddress)
    {

      addrH = 0xC0 | high8(address_);
      addrL = low8(address_);
    }
    else
    {
      addrH = 0;
      addrL = static_cast<uint8_t>(address_ & 0x7F);
    }
  }

  bool isLongAddress() const
  {
    return (addrH & 0xC0) == 0xC0;
  }

  uint16_t address() const
  {
    return to16(addrL, addrH & 0x3F);
  }
};

// QueryConsist

struct AllocateEngineToActivity : EngineMessage
{
  uint8_t allocationCode;

  AllocateEngineToActivity(uint8_t session_, uint8_t allocationCode_)
    : EngineMessage(OpCode::ALOC, session_)
    , allocationCode{allocationCode_}
  {
  }
};

struct SetEngineSessionMode : EngineMessage
{
  static constexpr uint8_t serviceModeBit = 2;
  static constexpr uint8_t soundControlModeBit = 3;

  enum SpeedMode : uint8_t
  {
    SpeedMode128 = 0b00,
    SpeedMode14 = 0b01,
    SpeedMode28WithInterleaveSteps = 0b10,
    SpeedMode28 = 0b11,
  };

  uint8_t mode;

  SetEngineSessionMode(uint8_t session_, SpeedMode speedMode_, bool serviceMode_, bool soundControlMode_)
    : EngineMessage(OpCode::STMOD, session_)
    , mode{0}
  {
    mode |= static_cast<uint8_t>(speedMode_);
    setBit<serviceModeBit>(mode, serviceMode_);
    setBit<soundControlModeBit>(mode, soundControlMode_);
  }

  SpeedMode speedMode() const
  {
    return static_cast<SpeedMode>(mode & 0x03);
  }

  bool serviceMode() const
  {
    return getBit<serviceModeBit>(mode);
  }

  bool soundControlMode() const
  {
    return getBit<soundControlModeBit>(mode);
  }
};

// ConsistEngine

// RemoveEngineFromConsist

struct SetEngineSpeedDirection : EngineMessage
{
  static constexpr uint8_t speedMask = 0x7F;
  static constexpr uint8_t directionBit = 7;

  uint8_t speedDir;

  SetEngineSpeedDirection(uint8_t session_, uint8_t speed_, bool directionForward_)
    : EngineMessage(OpCode::DSPD, session_)
    , speedDir(speed_ & speedMask)
  {
    setBit<directionBit>(speedDir, directionForward_);
  }

  uint8_t speed() const
  {
    return (speedDir & speedMask);
  }

  bool directionForward() const
  {
    return getBit<directionBit>(speedDir);
  }
};

// SetEngineFlags

struct SetEngineFunctionOn : EngineMessage
{
  uint8_t number;

  SetEngineFunctionOn(uint8_t session_, uint8_t number_)
    : EngineMessage(OpCode::DFNON, session_)
    , number{number_}
  {
    assert(number <= engineFunctionMax);
  }
};

struct SetEngineFunctionOff : EngineMessage
{
  uint8_t number;

  SetEngineFunctionOff(uint8_t session_, uint8_t number_)
    : EngineMessage(OpCode::DFNOF, session_)
    , number{number_}
  {
    assert(number <= engineFunctionMax);
  }
};

struct SetEngineFunctions : EngineMessage
{
  enum class Range : uint8_t
  {
    F0F4 = 1,
    F5F8 = 2,
    F9F12 = 3,
    F13F20 = 4,
    F21F28 = 5,
  };

  Range range;
  uint8_t value;

  std::span<const uint8_t> numbers() const
  {
    switch(range)
    {
      using enum Range;

      case F0F4:
      {
        static const std::array<uint8_t, 5> values{{0, 1, 2, 3, 4}};
        return values;
      }
      case F5F8:
      {
        static const std::array<uint8_t, 4> values{{5, 6, 7, 8}};
        return values;
      }
      case F9F12:
      {
        static const std::array<uint8_t, 4> values{{9, 10, 11, 12}};
        return values;
      }
      case F13F20:
      {
        static const std::array<uint8_t, 8> values{{13, 14, 15, 16, 17, 18, 19, 20}};
        return values;
      }
      case F21F28:
      {
        static const std::array<uint8_t, 8> values{{21, 22, 23, 24, 25, 26, 27, 28}};
        return values;
      }
    }
    assert(false);
    return {};
  }

protected:
  SetEngineFunctions(uint8_t session_, Range range_, uint8_t value_ = 0)
    : EngineMessage(OpCode::DFUN, session_)
    , range{range_}
    , value{value_}
  {
  }
};

struct SetEngineFunctionsF0F4 : SetEngineFunctions
{
  static constexpr uint8_t f0Bit = 4;
  static constexpr uint8_t f1Bit = 0;
  static constexpr uint8_t f2Bit = 1;
  static constexpr uint8_t f3Bit = 2;
  static constexpr uint8_t f4Bit = 3;

  SetEngineFunctionsF0F4(uint8_t session_, bool f0, bool f1, bool f2, bool f3, bool f4)
    : SetEngineFunctions(session_, Range::F0F4)
  {
    setBit<f0Bit>(value, f0);
    setBit<f1Bit>(value, f1);
    setBit<f2Bit>(value, f2);
    setBit<f3Bit>(value, f3);
    setBit<f4Bit>(value, f4);
  }

  bool f(uint8_t number) const
  {
    switch(number)
    {
      case 0: return f0();
      case 1: return f1();
      case 2: return f2();
      case 3: return f3();
      case 4: return f4();

      default: [[unlikely]]
        assert(false);
        return false;
    }
  }

  bool f0() const
  {
    return getBit<f0Bit>(value);
  }

  bool f1() const
  {
    return getBit<f1Bit>(value);
  }

  bool f2() const
  {
    return getBit<f2Bit>(value);
  }

  bool f3() const
  {
    return getBit<f3Bit>(value);
  }

  bool f4() const
  {
    return getBit<f4Bit>(value);
  }
};

struct SetEngineFunctionsF5F8 : SetEngineFunctions
{
  static constexpr uint8_t f5Bit = 0;
  static constexpr uint8_t f6Bit = 1;
  static constexpr uint8_t f7Bit = 2;
  static constexpr uint8_t f8Bit = 3;

  SetEngineFunctionsF5F8(uint8_t session_, bool f5, bool f6, bool f7, bool f8)
    : SetEngineFunctions(session_, Range::F5F8)
  {
    setBit<f5Bit>(value, f5);
    setBit<f6Bit>(value, f6);
    setBit<f7Bit>(value, f7);
    setBit<f8Bit>(value, f8);
  }

  bool f(uint8_t number) const
  {
    switch(number)
    {
      case 5: return f5();
      case 6: return f6();
      case 7: return f7();
      case 8: return f8();

      default: [[unlikely]]
        assert(false);
        return false;
    }
  }

  bool f5() const
  {
    return getBit<f5Bit>(value);
  }

  bool f6() const
  {
    return getBit<f6Bit>(value);
  }

  bool f7() const
  {
    return getBit<f7Bit>(value);
  }

  bool f8() const
  {
    return getBit<f8Bit>(value);
  }
};

struct SetEngineFunctionsF9F12 : SetEngineFunctions
{
  static constexpr uint8_t f9Bit  = 0;
  static constexpr uint8_t f10Bit = 1;
  static constexpr uint8_t f11Bit = 2;
  static constexpr uint8_t f12Bit = 3;

  SetEngineFunctionsF9F12(uint8_t session_, bool f9, bool f10, bool f11, bool f12)
    : SetEngineFunctions(session_, Range::F9F12)
  {
    setBit<f9Bit>(value,  f9);
    setBit<f10Bit>(value, f10);
    setBit<f11Bit>(value, f11);
    setBit<f12Bit>(value, f12);
  }

  bool f(uint8_t number) const
  {
    switch(number)
    {
      case 9:  return f9();
      case 10: return f10();
      case 11: return f11();
      case 12: return f12();

      default: [[unlikely]]
        assert(false);
        return false;
    }
  }

  bool f9()  const
  {
    return getBit<f9Bit>(value);
  }

  bool f10() const
  {
    return getBit<f10Bit>(value);
  }

  bool f11() const
  {
    return getBit<f11Bit>(value);
  }

  bool f12() const
  {
    return getBit<f12Bit>(value);
  }
};

struct SetEngineFunctionsF13F20 : SetEngineFunctions
{
  static constexpr uint8_t f13Bit = 0;
  static constexpr uint8_t f14Bit = 1;
  static constexpr uint8_t f15Bit = 2;
  static constexpr uint8_t f16Bit = 3;
  static constexpr uint8_t f17Bit = 4;
  static constexpr uint8_t f18Bit = 5;
  static constexpr uint8_t f19Bit = 6;
  static constexpr uint8_t f20Bit = 7;

  SetEngineFunctionsF13F20(uint8_t session_, bool f13, bool f14, bool f15, bool f16, bool f17, bool f18, bool f19, bool f20)
    : SetEngineFunctions(session_, Range::F13F20)
  {
    setBit<f13Bit>(value, f13);
    setBit<f14Bit>(value, f14);
    setBit<f15Bit>(value, f15);
    setBit<f16Bit>(value, f16);
    setBit<f17Bit>(value, f17);
    setBit<f18Bit>(value, f18);
    setBit<f19Bit>(value, f19);
    setBit<f20Bit>(value, f20);
  }

  bool f(uint8_t number) const
  {
    switch(number)
    {
      case 13: return f13();
      case 14: return f14();
      case 15: return f15();
      case 16: return f16();
      case 17: return f17();
      case 18: return f18();
      case 19: return f19();
      case 20: return f20();

      default: [[unlikely]]
        assert(false);
        return false;
    }
  }

  bool f13() const
  {
    return getBit<f13Bit>(value);
  }

  bool f14() const
  {
    return getBit<f14Bit>(value);
  }

  bool f15() const
  {
    return getBit<f15Bit>(value);
  }

  bool f16() const
  {
    return getBit<f16Bit>(value);
  }

  bool f17() const
  {
    return getBit<f17Bit>(value);
  }

  bool f18() const
  {
    return getBit<f18Bit>(value);
  }

  bool f19() const
  {
    return getBit<f19Bit>(value);
  }

  bool f20() const
  {
    return getBit<f20Bit>(value);
  }
};

struct SetEngineFunctionsF21F28 : SetEngineFunctions
{
  static constexpr uint8_t f21Bit = 0;
  static constexpr uint8_t f22Bit = 1;
  static constexpr uint8_t f23Bit = 2;
  static constexpr uint8_t f24Bit = 3;
  static constexpr uint8_t f25Bit = 4;
  static constexpr uint8_t f26Bit = 5;
  static constexpr uint8_t f27Bit = 6;
  static constexpr uint8_t f28Bit = 7;

  SetEngineFunctionsF21F28(uint8_t session_, bool f21, bool f22, bool f23, bool f24, bool f25, bool f26, bool f27, bool f28)
    : SetEngineFunctions(session_, Range::F21F28)
  {
    setBit<f21Bit>(value, f21);
    setBit<f22Bit>(value, f22);
    setBit<f23Bit>(value, f23);
    setBit<f24Bit>(value, f24);
    setBit<f25Bit>(value, f25);
    setBit<f26Bit>(value, f26);
    setBit<f27Bit>(value, f27);
    setBit<f28Bit>(value, f28);
  }

  bool f(uint8_t number) const
  {
    switch(number)
    {
      case 21: return f21();
      case 22: return f22();
      case 23: return f23();
      case 24: return f24();
      case 25: return f25();
      case 26: return f26();
      case 27: return f27();
      case 28: return f28();

      default: [[unlikely]]
        assert(false);
        return false;
    }
  }

  bool f21() const
  {
    return getBit<f21Bit>(value);
  }

  bool f22() const
  {
    return getBit<f22Bit>(value);
  }

  bool f23() const
  {
    return getBit<f23Bit>(value);
  }

  bool f24() const
  {
    return getBit<f24Bit>(value);
  }

  bool f25() const
  {
    return getBit<f25Bit>(value);
  }

  bool f26() const
  {
    return getBit<f26Bit>(value);
  }

  bool f27() const
  {
    return getBit<f27Bit>(value);
  }

  bool f28() const
  {
    return getBit<f28Bit>(value);
  }
};

struct GetEngineSession : Message
{
  static constexpr uint8_t modeMask = 0b11;

  enum class Mode : uint8_t
  {
    Request = 0b00,
    Steal = 0b01,
    Share = 0b10,
  };

  uint8_t addrH;
  uint8_t addrL;
  uint8_t flags;

  GetEngineSession(uint16_t address_, bool longAddress_, Mode mode_)
    : Message(OpCode::GLOC)
    , flags(static_cast<uint8_t>(mode_) & modeMask)
  {
    assert(mode_ == Mode::Request || mode_ == Mode::Steal || mode_ == Mode::Share);

    if(longAddress_)
    {
      addrH = 0xC0 | high8(address_);
      addrL = low8(address_);
    }
    else
    {
      addrH = 0;
      addrL = static_cast<uint8_t>(address_ & 0x7F);
    }
  }

  bool isLongAddress() const
  {
    return (addrH & 0xC0) == 0xC0;
  }

  uint16_t address() const
  {
    return to16(addrL, addrH & 0x3F);
  }

  Mode mode() const
  {
    return static_cast<Mode>(flags & modeMask);
  }
};

struct EngineReport : EngineMessage
{
  static constexpr uint8_t speedMask = 0x7F;
  static constexpr uint8_t directionBit = 7;
  static constexpr uint8_t f0Bit = 4;
  static constexpr uint8_t f1Bit = 0;
  static constexpr uint8_t f2Bit = 1;
  static constexpr uint8_t f3Bit = 2;
  static constexpr uint8_t f4Bit = 3;
  static constexpr uint8_t f5Bit = 0;
  static constexpr uint8_t f6Bit = 1;
  static constexpr uint8_t f7Bit = 2;
  static constexpr uint8_t f8Bit = 3;
  static constexpr uint8_t f9Bit = 0;
  static constexpr uint8_t f10Bit = 1;
  static constexpr uint8_t f11Bit = 2;
  static constexpr uint8_t f12Bit = 3;

  uint8_t addrH;
  uint8_t addrL;
  uint8_t speedDir;
  uint8_t f0f4;
  uint8_t f5f8;
  uint8_t f9f12;

  EngineReport(
        uint8_t session_, uint16_t address_, bool longAddress_,
        uint8_t speed_, bool directionForward_,
        bool f0, bool f1, bool f2, bool f3, bool f4,
        bool f5, bool f6, bool f7, bool f8,
        bool f9, bool f10, bool f11, bool f12)

    : EngineMessage(OpCode::PLOC, session_)
    , speedDir(speed_ & speedMask)
    , f0f4{0}
    , f5f8{0}
    , f9f12{0}
  {
    if(longAddress_)
    {
      addrH = 0xC0 | high8(address_);
      addrL = low8(address_);
    }
    else
    {
      addrH = 0;
      addrL = static_cast<uint8_t>(address_ & 0x7F);
    }

    setBit<directionBit>(speedDir, directionForward_);

    setBit<f0Bit>(f0f4, f0);
    setBit<f1Bit>(f0f4, f1);
    setBit<f2Bit>(f0f4, f2);
    setBit<f3Bit>(f0f4, f3);
    setBit<f4Bit>(f0f4, f4);
    setBit<f5Bit>(f5f8, f5);
    setBit<f6Bit>(f5f8, f6);
    setBit<f7Bit>(f5f8, f7);
    setBit<f8Bit>(f5f8, f8);
    setBit<f9Bit>(f9f12, f9);
    setBit<f10Bit>(f9f12, f10);
    setBit<f11Bit>(f9f12, f11);
    setBit<f12Bit>(f9f12, f12);
  }

  bool isLongAddress() const
  {
    return (addrH & 0xC0) == 0xC0;
  }

  uint16_t address() const
  {
    return to16(addrL, addrH & 0x3F);
  }

  uint8_t speed() const
  {
    return (speedDir & speedMask);
  }

  bool directionForward() const
  {
    return getBit<directionBit>(speedDir);
  }

  bool f(uint8_t number) const
  {
    switch(number)
    {
      case 0: return f0();
      case 1: return f1();
      case 2: return f2();
      case 3: return f3();
      case 4: return f4();
      case 5: return f5();
      case 6: return f6();
      case 7: return f7();
      case 8: return f8();
      case 9:  return f9();
      case 10: return f10();
      case 11: return f11();
      case 12: return f12();

      default: [[unlikely]]
        assert(false);
        return false;
    }
  }

  bool f0() const
  {
    return getBit<f0Bit>(f0f4);
  }

  bool f1() const
  {
    return getBit<f1Bit>(f0f4);
  }

  bool f2() const
  {
    return getBit<f2Bit>(f0f4);
  }

  bool f3() const
  {
    return getBit<f3Bit>(f0f4);
  }

  bool f4() const
  {
    return getBit<f4Bit>(f0f4);
  }

  bool f5() const
  {
    return getBit<f5Bit>(f5f8);
  }

  bool f6() const
  {
    return getBit<f6Bit>(f5f8);
  }

  bool f7() const
  {
    return getBit<f7Bit>(f5f8);
  }

  bool f8() const
  {
    return getBit<f8Bit>(f5f8);
  }

  bool f9()  const
  {
    return getBit<f9Bit>(f9f12);
  }

  bool f10() const
  {
    return getBit<f10Bit>(f9f12);
  }

  bool f11() const
  {
    return getBit<f11Bit>(f9f12);
  }

  bool f12() const
  {
    return getBit<f12Bit>(f9f12);
  }
};

}

#endif
