/**
 * server/src/hardware/protocol/xpressnet.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020 Reinder Feenstra
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

#include "xpressnet.hpp"
#include "../../core/traintastic.hpp"
#include "../decoder/decoder.hpp"

namespace Protocol {

uint8_t XpressNet::calcChecksum(const void* msg)
{
  assert(msg);
  return calcChecksum(*reinterpret_cast<const Message*>(msg));
}

uint8_t XpressNet::calcChecksum(const Message& msg)
{
  const uint8_t* p = reinterpret_cast<const uint8_t*>(&msg);
  const int dataSize = msg.dataSize();
  uint8_t checksum = p[0];
  for(int i = 1; i <= dataSize; i++)
    checksum ^= p[i];
  return checksum;
}

bool XpressNet::isChecksumValid(const Message& msg)
{
  return calcChecksum(msg) == *(reinterpret_cast<const uint8_t*>(&msg) + msg.dataSize() + 1);
}

XpressNet::XpressNet(Object& parent, const std::string& parentPropertyName, std::function<bool(const Message&)> send) :
  SubObject(parent, parentPropertyName),
  m_send{std::move(send)},
  commandStation{this, "command_station", XpressNetCommandStation::Custom, PropertyFlags::ReadWrite,
    [this](XpressNetCommandStation value)
    {
      switch(value)
      {
        case XpressNetCommandStation::Custom:
          break;

        case XpressNetCommandStation::Roco10764:
          useEmergencyStopLocomotiveCommand.setValueInternal(false);
          useFunctionStateCommands.setValueInternal(false);
          useRocoF13F20Command.setValueInternal(true);
          break;
      }
    }},
  useEmergencyStopLocomotiveCommand{this, "use_emergency_stop_locomotive_command", false, PropertyFlags::ReadWrite,
    [this](bool)
    {
      commandStation = XpressNetCommandStation::Custom;
    }},
  useFunctionStateCommands{this, "use_function_state_commands", false, PropertyFlags::ReadWrite,
    [this](bool)
    {
      commandStation = XpressNetCommandStation::Custom;
    }},
  useRocoF13F20Command{this, "use_roco_f13_f20_command", false, PropertyFlags::ReadWrite,
    [this](bool)
    {
      commandStation = XpressNetCommandStation::Custom;
    }}
{
  m_interfaceItems.add(commandStation)
    .addAttributeEnabled(false);
  m_interfaceItems.add(useEmergencyStopLocomotiveCommand)
    .addAttributeEnabled(false);
  m_interfaceItems.add(useFunctionStateCommands)
    .addAttributeEnabled(false);
  m_interfaceItems.add(useRocoF13F20Command)
    .addAttributeEnabled(false);
}

void XpressNet::worldEvent(WorldState state, WorldEvent event)
{
  SubObject::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  commandStation.setAttributeEnabled(editable);
  useEmergencyStopLocomotiveCommand.setAttributeEnabled(editable);
  useFunctionStateCommands.setAttributeEnabled(editable);
  useRocoF13F20Command.setAttributeEnabled(editable);
}

void XpressNet::decoderChanged(const Hardware::Decoder& decoder, Hardware::DecoderChangeFlags changes, uint32_t functionNumber)
{
  using namespace Hardware;

  Traintastic::instance->console->debug(id(), "XpressNet::decoderChanged");

  if(useEmergencyStopLocomotiveCommand && changes == DecoderChangeFlags::EmergencyStop && decoder.emergencyStop)
    send(EmergencyStopLocomotive(
          decoder.address,
          decoder.longAddress));
  else if(has(changes, DecoderChangeFlags::EmergencyStop | DecoderChangeFlags::Direction | DecoderChangeFlags::SpeedStep | DecoderChangeFlags::SpeedSteps))
  {
    switch(decoder.speedSteps)
    {
      case 14:
        send(SpeedAndDirectionInstruction14(
          decoder.address,
          decoder.longAddress,
          decoder.emergencyStop,
          decoder.direction,
          decoder.speedStep,
          getFunctionValue(decoder, 0)));
        break;

      case 27:
        send(SpeedAndDirectionInstruction27(
          decoder.address,
          decoder.longAddress,
          decoder.emergencyStop,
          decoder.direction,
          decoder.speedStep));
        break;

      case 28:
        send(SpeedAndDirectionInstruction28(
          decoder.address,
          decoder.longAddress,
          decoder.emergencyStop,
          decoder.direction,
          decoder.speedStep));
        break;

      case 126:
      case 128:
        send(SpeedAndDirectionInstruction128(
          decoder.address,
          decoder.longAddress,
          decoder.emergencyStop,
          decoder.direction,
          decoder.speedStep));
        break;

      default:
        Traintastic::instance->console->warning(id(), std::to_string(decoder.speedSteps) + " speed steps not supported");
        break;
    }
  }
  else if(has(changes, DecoderChangeFlags::FunctionValue))
  {
    if(functionNumber <= 4)
      send(FunctionInstructionGroup1(
        decoder.address,
        decoder.longAddress,
        getFunctionValue(decoder, 0),
        getFunctionValue(decoder, 1),
        getFunctionValue(decoder, 2),
        getFunctionValue(decoder, 3),
        getFunctionValue(decoder, 4)));
    else if(functionNumber <= 8)
      send(FunctionInstructionGroup2(
        decoder.address,
        decoder.longAddress,
        getFunctionValue(decoder, 5),
        getFunctionValue(decoder, 6),
        getFunctionValue(decoder, 7),
        getFunctionValue(decoder, 8)));
    else if(functionNumber <= 12)
      send(FunctionInstructionGroup3(
        decoder.address,
        decoder.longAddress,
        getFunctionValue(decoder, 9),
        getFunctionValue(decoder, 10),
        getFunctionValue(decoder, 11),
        getFunctionValue(decoder, 12)));
    else if(useRocoF13F20Command && functionNumber <= 20)
      send(RocoFunctionInstructionF13F20(
        decoder.address,
        decoder.longAddress,
        getFunctionValue(decoder, 13),
        getFunctionValue(decoder, 14),
        getFunctionValue(decoder, 15),
        getFunctionValue(decoder, 16),
        getFunctionValue(decoder, 17),
        getFunctionValue(decoder, 18),
        getFunctionValue(decoder, 19),
        getFunctionValue(decoder, 20)));
    else
      Traintastic::instance->console->warning(id(), "Function F" + std::to_string(functionNumber) + " not supported");
  }
}

bool XpressNet::getFunctionValue(const Hardware::Decoder& decoder, uint32_t number)
{
  const auto& f = decoder.getFunction(number);
  return f && f->value;
}

}
