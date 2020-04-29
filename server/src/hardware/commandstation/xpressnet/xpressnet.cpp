/**
 * server/src/hardware/commandstation/xpressnet/xpressnet.cpp
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

#include "xpressnet.hpp"
#include "../../../core/traintastic.hpp"
#include "../../decoder/decoder.hpp"
#include "../../decoder/decoderlist.hpp"
#include "../../decoder/decoderchangeflags.hpp"

namespace Hardware::CommandStation {

XpressNet::XpressNet(const std::weak_ptr<World>& world, std::string_view _id) :
  CommandStation(world, _id),
  /*emergencyStop{this, "emergency_stop", false, PropertyFlags::ReadWrite | PropertyFlags::StoreState,
    [this](bool value)
    {
      if(online)
      {
        if(value)
          send(EmergencyStop());
        else
          send(NormalOperationResumed());
      }
    }},
  trackVoltageOff{this, "track_voltage_off", false, PropertyFlags::ReadWrite | PropertyFlags::StoreState,
    [this](bool value)
    {
      if(online)
      {
        if(value)
          send(TrackPowerOff());
        else
          send(NormalOperationResumed());
      }
    }},*/
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
  m_interfaceItems.add(emergencyStop)
    .addAttributeObjectEditor(false);
  m_interfaceItems.add(trackVoltageOff)
    .addAttributeObjectEditor(false);
  m_interfaceItems.insertBefore(commandStation, notes)
    .addAttributeValues(XpressNetCommandStationValues)
    .addAttributeCategory(Category::XpressNet)
    .addAttributeEnabled(false);
  m_interfaceItems.insertBefore(useEmergencyStopLocomotiveCommand, notes)
    .addAttributeCategory(Category::XpressNet)
    .addAttributeEnabled(false);
  m_interfaceItems.insertBefore(useFunctionStateCommands, notes)
    .addAttributeCategory(Category::XpressNet)
    .addAttributeEnabled(false);
  m_interfaceItems.insertBefore(useRocoF13F20Command, notes)
    .addAttributeCategory(Category::XpressNet)
    .addAttributeEnabled(false);
}

void XpressNet::worldEvent(WorldState state, WorldEvent event)
{
  CommandStation::worldEvent(state, event);

  commandStation.setAttributeEnabled(mode == TraintasticMode::Edit);
  useEmergencyStopLocomotiveCommand.setAttributeEnabled(mode == TraintasticMode::Edit);
  useFunctionStateCommands.setAttributeEnabled(mode == TraintasticMode::Edit);
  useRocoF13F20Command.setAttributeEnabled(mode == TraintasticMode::Edit);
}

void XpressNet::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(!online)
    return;

  Traintastic::instance->console->debug(id, "XpressNet::decoderChanged");

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
        Traintastic::instance->console->warning(id, std::to_string(decoder.speedSteps) + " speed steps not supported");
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
      send(Roco::FunctionInstructionF13F20(
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
      Traintastic::instance->console->warning(id, "Function F" + std::to_string(functionNumber) + " not supported");
  }
}

bool XpressNet::getFunctionValue(const Decoder& decoder, uint32_t number)
{
  const auto& f = decoder.getFunction(number);
  return f && f->value;
}

}
