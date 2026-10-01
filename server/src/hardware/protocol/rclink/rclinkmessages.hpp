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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_RCLINK_RCLINKMESSAGES_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_RCLINK_RCLINKMESSAGES_HPP

#include <cassert>
#include <cstdint>
#include <string>
#include <span>
#include "../../../utils/bit.hpp"
#include "../../../utils/byte.hpp"

namespace RCLink {

enum class Command : uint8_t
{
  InitializeAll = 0x20,
  DisableDataTraffic = 0x21,
  QueryDetectorBase = 0x40,
  QueryInfo = 0x60,
  ProgramDetectorBase = 0x80,
  RequestDiagnosticDataBase = 0xA0,
  ScopeMode = 0xC0,
  SystemOff = 0xE0,
  Detector = 0xFC,
  Info = 0xFD,
  CV = 0xFE,
};

struct Message
{
  Command command;

protected:
  Message(Command cmd)
    : command{cmd}
  {
  }
};

struct AddressMessage : Message
{
  uint8_t address() const
  {
    return static_cast<uint8_t>(command) & addressMask;
  }

protected:
  static constexpr uint8_t addressMask = 0x1F;

  AddressMessage(Command base, uint8_t address)
    : Message(static_cast<Command>(static_cast<uint8_t>(base) + (address & addressMask)))
  {
    assert(address >= 1 && address <= 24);
  }
};

struct InitializeAll : Message
{
  InitializeAll()
    : Message(Command::InitializeAll)
  {
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(InitializeAll)) &&
      (message[0] == static_cast<uint8_t>(Command::InitializeAll));
  }
};
static_assert(sizeof(InitializeAll) == 1);

struct DisableDataTraffic : Message
{
  DisableDataTraffic()
    : Message(Command::DisableDataTraffic)
  {
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(DisableDataTraffic)) &&
      (message[0] == static_cast<uint8_t>(Command::DisableDataTraffic));
  }
};
static_assert(sizeof(DisableDataTraffic) == 1);

struct QueryDetector : AddressMessage
{
  explicit QueryDetector(uint8_t address)
    : AddressMessage(Command::QueryDetectorBase, address)
  {
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(QueryDetector)) &&
      ((message[0] & ~addressMask) == static_cast<uint8_t>(Command::QueryDetectorBase));
  }
};
static_assert(sizeof(QueryDetector) == 1);


struct QueryInfo : Message
{
  QueryInfo()
    : Message(Command::QueryInfo)
  {
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(QueryInfo)) &&
      (message[0] == static_cast<uint8_t>(Command::QueryInfo));
  }
};
static_assert(sizeof(QueryInfo) == 1);

struct ProgramDetector : AddressMessage
{
  explicit ProgramDetector(uint8_t address)
    : AddressMessage(Command::ProgramDetectorBase, address)
  {
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(ProgramDetector)) &&
      ((message[0] & ~addressMask) == static_cast<uint8_t>(Command::ProgramDetectorBase));
  }
};
static_assert(sizeof(ProgramDetector) == 1);

struct RequestDiagnosticData : AddressMessage
{
  explicit RequestDiagnosticData(uint8_t address)
    : AddressMessage(Command::RequestDiagnosticDataBase, address)
  {
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(RequestDiagnosticData)) &&
      ((message[0] & ~addressMask) == static_cast<uint8_t>(Command::RequestDiagnosticDataBase));
  }
};
static_assert(sizeof(RequestDiagnosticData) == 1);

struct ScopeMode : Message
{
  ScopeMode()
    : Message(Command::ScopeMode)
  {
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(ScopeMode)) &&
      (message[0] == static_cast<uint8_t>(Command::ScopeMode));
  }
};
static_assert(sizeof(ScopeMode) == 1);

struct SystemOff : Message
{
  SystemOff()
    : Message(Command::SystemOff)
  {
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(SystemOff)) &&
      (message[0] == static_cast<uint8_t>(Command::SystemOff));
  }
};
static_assert(sizeof(SystemOff) == 1);

struct Detector : Message
{
  static constexpr uint8_t orientationBit = 7;
  static constexpr uint8_t railcomAddressHighMask = static_cast<uint8_t>(~(1 << orientationBit));
  static constexpr uint16_t railcomNoAddress = 0x7777;

  uint8_t address;
  uint8_t railcomAddressHigh;
  uint8_t railcomAddressLow;

  Detector(uint8_t addr, uint16_t railcomAddr, bool orientation_)
    : Message(Command::Detector)
    , address{addr}
    , railcomAddressHigh(high8(railcomAddr) & railcomAddressHighMask)
    , railcomAddressLow{low8(railcomAddr)}
  {
    assert(addr >= 1 && addr <= 24);
    setBit<orientationBit>(railcomAddressHigh, orientation_);
  }

  bool occupied() const
  {
    return railcomAddress() != 0;
  }

  bool orientation() const
  {
    return getBit<orientationBit>(railcomAddressHigh);
  }

  bool hasRailcomAddress() const
  {
    return occupied() && railcomAddress() != railcomNoAddress;
  }

  uint16_t railcomAddress() const
  {
    return to16(railcomAddressLow, railcomAddressHigh & railcomAddressHighMask);
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(Detector)) &&
      (message[0] == static_cast<uint8_t>(Command::Detector));
  }
};
static_assert(sizeof(Detector) == 4);

struct Info : Message
{
  uint8_t serialNumberHigh;
  uint8_t serialNumberLow;
  uint8_t softwareVersion;
  uint8_t hardwareVersion;

  Info(uint16_t serialNumber_, uint8_t softwareVersion_, uint8_t hardwareVersion_)
    : Message(Command::Info)
    , serialNumberHigh{high8(serialNumber_)}
    , serialNumberLow{low8(serialNumber_)}
    , softwareVersion{softwareVersion_}
    , hardwareVersion{hardwareVersion_}
  {
  }

  uint16_t serialNumber() const
  {
    return to16(serialNumberLow, serialNumberHigh);
  }

  uint8_t softwareVersionMajor() const
  {
    return softwareVersion / 10;
  }

  uint8_t softwareVersionMinor() const
  {
    return softwareVersion % 10;
  }

  uint8_t hardwareVersionMajor() const
  {
    return hardwareVersion / 10;
  }

  uint8_t hardwareVersionMinor() const
  {
    return hardwareVersion % 10;
  }

  static bool check(std::span<const uint8_t> message)
  {
    return
      (message.size() == sizeof(Info)) &&
      (message[0] == static_cast<uint8_t>(Command::Info));
  }
};
static_assert(sizeof(Info) == 5);

std::string toString(std::span<const uint8_t> message);

}

#endif
