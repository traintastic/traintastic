/**
 * server/src/hardware/commandstation/protocol/xpressnet.cpp
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

/**
 * hardware/protocol/xpressnet.cpp - XpressNet protocol
 *
 * This file is part of the traintastic-server source code
 *
 * Copyright (C) 2019-2020 Reinder Feenstra <reinderfeenstra@gmail.com>
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
#if 0
#include "xpressnet.hpp"
#include "../commandstation.hpp"
#include "../../decoder/decoder.hpp"
#include "../../decoder/decoderchangeflags.hpp"
#include "../../decoder/decoderfunction.hpp"



#include "../../../core/traintastic.hpp"


namespace Hardware::CommandStation::Protocol {

inline void addressLowHigh(uint16_t address, uint8_t& addressLow, uint8_t& addressHigh)
{
  addressLow = address & 0xff;
  if(address <= 99)
    addressHigh = 0x00;
  else
    addressHigh = (0xC0 | address >> 8);
}

struct EmergencyStopLocomotive
{
  const uint8_t headerByte = 0x92;
  uint8_t addressHigh;
  uint8_t addressLow;
  uint8_t checksum;

  EmergencyStopLocomotive(uint16_t address)
  {
    addressLowHigh(address, addressLow, addressHigh);
  }
} __attribute__((packed));

struct SpeedAndDirectionInstruction
{
  const uint8_t headerByte = 0xE4;
  uint8_t identification;
  uint8_t addressHigh;
  uint8_t addressLow;
  uint8_t speedAndDirection;
  uint8_t checksum;

  SpeedAndDirectionInstruction(uint16_t address)
  {
    addressLowHigh(address, addressLow, addressHigh);
  }
} __attribute__((packed));

struct FunctionInstructionGroup
{
  const uint8_t headerByte = 0xE4;
  uint8_t identification;
  uint8_t addressHigh;
  uint8_t addressLow;
  uint8_t functions = 0x00;
  uint8_t checksum;

  FunctionInstructionGroup(uint8_t group, uint16_t address)
  {
    assert(group >= 1 && group <= 3);
    identification = 0x1f + group;
    addressLowHigh(address, addressLow, addressHigh);
  }
} __attribute__((packed));

struct SetFunctionStateGroup
{
  const uint8_t headerByte = 0xE4;
  uint8_t identification;
  uint8_t addressHigh;
  uint8_t addressLow;
  uint8_t state = 0x00;
  uint8_t checksum;

  SetFunctionStateGroup(uint8_t group, uint16_t address)
  {
    assert(group >= 1 && group <= 3);
    identification = 0x23 + group;
    addressLowHigh(address, addressLow, addressHigh);
  }
} __attribute__((packed));

struct RocoSetFunctionStateF13F20
{
  const uint8_t headerByte = 0xE4;
  const uint8_t identification = 0xF3;
  uint8_t addressHigh;
  uint8_t addressLow;
  uint8_t state = 0x00;
  uint8_t checksum;

  RocoSetFunctionStateF13F20(uint16_t address)
  {
    addressLowHigh(address, addressLow, addressHigh);
  }
} __attribute__((packed));

XpressNet::XpressNet(const std::weak_ptr<World>& world, const std::string& id, std::function<void(const void*)>&& send) :
  IdObject(world, id),
  m_send{std::move(send)},
  commandStation{this, "command_station", XpressNetCommandStation::Custom, PropertyFlags::ReadWrite},
  useFunctionStateCommands{this, "use_function_state_commands", false, PropertyFlags::ReadWrite},
  useRocoF13F20Command{this, "use_roco_f13_f20_command", false, PropertyFlags::ReadWrite}
{
  assert(m_send);
  m_interfaceItems.add(commandStation);
  m_interfaceItems.add(useFunctionStateCommands);
}

bool XpressNet::isDecoderSupported(const Decoder& decoder) const
{
  return
    decoder.protocol == DecoderProtocol::DCC &&
    decoder.address >= addressMin &&
    decoder.address <= addressMax;
}


uint8_t XpressNet::calcChecksum(const void* cmd)
{
  const uint8_t* p = static_cast<const uint8_t*>(cmd);
  const int length = p[0] & 0x0f;
  uint8_t checksum = p[0];
  for(int i = 1; i <= length; i++)
    checksum ^= p[i];
  return checksum;
}

void XpressNet::sendEmergencyStop(const Decoder& decoder)
{
  Traintastic::instance->console->debug(id, "XpressNet::sendEmergencyStop");

  EmergencyStopLocomotive cmd(decoder.address);
  cmd.checksum = calcChecksum(&cmd);
  m_send(&cmd);
}

void XpressNet::sendSpeedAndDirectionInstruction(const Decoder& decoder)
{
  Traintastic::instance->console->debug(id, "XpressNet::sendSpeedAndDirectionInstruction");

  SpeedAndDirectionInstruction cmd(decoder.address);

  assert(decoder.speedStep <= decoder.speedSteps);
  switch(decoder.speedSteps)
  {
    case 14:
      cmd.identification = 0x10;
      if(decoder.emergencyStop)
        cmd.speedAndDirection = 0x01;
      else if(decoder.speedStep > 0)
        cmd.speedAndDirection = decoder.speedStep + 1;
      break;

    case 27:
      cmd.identification = 0x11;
      if(decoder.emergencyStop)
        cmd.speedAndDirection = 0x01;
      else if(decoder.speedStep > 0)
      {
        const uint8_t speedStep = decoder.speedStep + 1;
        cmd.speedAndDirection = ((speedStep & 0x01) << 4) | (speedStep >> 1);
      }
      break;

    case 28:
      cmd.identification = 0x12;
      if(decoder.emergencyStop)
        cmd.speedAndDirection = 0x01;
      else if(decoder.speedStep > 0)
      {
        const uint8_t speedStep = decoder.speedStep + 1;
        cmd.speedAndDirection = ((speedStep & 0x01) << 4) | (speedStep >> 1);
      }
      break;

    case 126:
      cmd.identification = 0x13;
      if(decoder.emergencyStop)
        cmd.speedAndDirection = 0x01;
      else if(decoder.speedStep > 0)
        cmd.speedAndDirection = decoder.speedStep + 1;
      break;
  }

  if(decoder.direction == Direction::Forward)
    cmd.speedAndDirection |= 0x80;

  cmd.checksum = calcChecksum(&cmd);

  m_send(&cmd);
}

void XpressNet::sendFunctionInstructionGroup1(const Decoder& decoder)
{
  FunctionInstructionGroup cmd(1, decoder.address);

  // FL/F0:
  const std::shared_ptr<DecoderFunction>& f = decoder.getFunction(0);
  if(f && f->value)
    cmd.functions |= 0x10;

  // F1 .. F4:
  for(uint32_t i = 1; i <= 4; i++)
  {
    const std::shared_ptr<DecoderFunction>& f = decoder.getFunction(i);
    if(f && f->value)
      cmd.functions |= 1 << (i - 1);
  }

  cmd.checksum = calcChecksum(&cmd);

  m_send(&cmd);
}

void XpressNet::sendFunctionInstructionGroup2(const Decoder& decoder)
{
  FunctionInstructionGroup cmd(2, decoder.address);

  // F5 .. F8:
  for(uint32_t i = 5; i <= 8; i++)
  {
    const std::shared_ptr<DecoderFunction>& f = decoder.getFunction(i);
    if(f && f->value)
      cmd.functions |= 1 << (i - 5);
  }

  cmd.checksum = calcChecksum(&cmd);

  m_send(&cmd);
}

void XpressNet::sendFunctionInstructionGroup3(const Decoder& decoder)
{
  FunctionInstructionGroup cmd(3, decoder.address);

  // F9 .. F12:
  for(uint32_t i = 9; i <= 12; i++)
  {
    const std::shared_ptr<DecoderFunction>& f = decoder.getFunction(i);
    if(f && f->value)
      cmd.functions |= 1 << (i - 9);
  }

  cmd.checksum = calcChecksum(&cmd);

  m_send(&cmd);
}

void XpressNet::sendSetFunctionStateGroup1(const Decoder& decoder)
{
  SetFunctionStateGroup cmd(1, decoder.address);

  // FL/F0:
  const std::shared_ptr<DecoderFunction>& f = decoder.getFunction(0);
  if(f && f->momentary)
    cmd.state |= 0x10;

  // F1 .. F4:
  for(uint32_t i = 1; i <= 4; i++)
  {
    const std::shared_ptr<DecoderFunction>& f = decoder.getFunction(i);
    if(f && f->momentary)
      cmd.state |= 1 << (i - 1);
  }

  cmd.checksum = calcChecksum(&cmd);

  m_send(&cmd);
}

void XpressNet::sendSetFunctionStateGroup2(const Decoder& decoder)
{
  SetFunctionStateGroup cmd(2, decoder.address);

  // F5 .. F8:
  for(uint32_t i = 5; i <= 8; i++)
  {
    const std::shared_ptr<DecoderFunction>& f = decoder.getFunction(i);
    if(f && f->momentary)
      cmd.state |= 1 << (i - 5);
  }

  cmd.checksum = calcChecksum(&cmd);

  m_send(&cmd);
}

void XpressNet::sendSetFunctionStateGroup3(const Decoder& decoder)
{
  SetFunctionStateGroup cmd(3, decoder.address);

  // F9 .. F12:
  for(uint32_t i = 9; i <= 12; i++)
  {
    const std::shared_ptr<DecoderFunction>& f = decoder.getFunction(i);
    if(f && f->momentary)
      cmd.state |= 1 << (i - 9);
  }

  cmd.checksum = calcChecksum(&cmd);

  m_send(&cmd);
}

void XpressNet::sendRocoSetFunctionStateF13F20(const Decoder& decoder)
{
  RocoSetFunctionStateF13F20 cmd(decoder.address);

  // F13 .. F20:
  for(uint32_t i = 13; i <= 20; i++)
  {
    const std::shared_ptr<DecoderFunction>& f = decoder.getFunction(i);
    if(f && f->value)
      cmd.state |= 1 << (i - 13);
  }

  cmd.checksum = calcChecksum(&cmd);

  m_send(&cmd);
}

}
#endif
