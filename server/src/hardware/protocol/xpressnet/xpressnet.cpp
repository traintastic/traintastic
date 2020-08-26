/**
 * server/src/hardware/protocol/xpressnet/xpressnet.cpp
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
#include "../../../core/eventloop.hpp"
#include "../../decoder/decoder.hpp"
#include "../../../core/attributes.hpp"

namespace XpressNet {

XpressNet::XpressNet(Object& _parent, const std::string& parentPropertyName, std::function<bool(const Message&)> send) :
  SubObject(_parent, parentPropertyName),
  m_commandStation{dynamic_cast<CommandStation*>(&_parent)},
  m_send{std::move(send)},
  m_debugLog{true},
  commandStation{this, "command_station", XpressNetCommandStation::Custom, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](XpressNetCommandStation value)
    {
      switch(value)
      {
        case XpressNetCommandStation::Custom:
          break;

        case XpressNetCommandStation::Roco10764:
          useEmergencyStopLocomotiveCommand.setValueInternal(false);
          //useFunctionStateCommands.setValueInternal(false);
          useRocoF13F20Command.setValueInternal(true);
          break;

        case XpressNetCommandStation::DigikeijsDR5000:
          useEmergencyStopLocomotiveCommand.setValueInternal(false); // ?????
          //useFunctionStateCommands.setValueInternal(false);
          useRocoF13F20Command.setValueInternal(true);
          break;
      }
      // TODO: updateEnabled();
    }},
  useEmergencyStopLocomotiveCommand{this, "use_emergency_stop_locomotive_command", false, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](bool)
    {
      commandStation = XpressNetCommandStation::Custom;
    }},
  //useFunctionStateCommands{this, "use_function_state_commands", false, PropertyFlags::ReadWrite | PropertyFlags::Store,
  //  [this](bool)
  //  {
  //    commandStation = XpressNetCommandStation::Custom;
  //  }},
  useRocoF13F20Command{this, "use_roco_f13_f20_command", false, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](bool)
    {
      commandStation = XpressNetCommandStation::Custom;
    }},
  debugLog{this, "debug_log", m_debugLog, PropertyFlags::ReadWrite | PropertyFlags::Store,
   [this](bool value)
    {
      m_debugLog = value;
    }}
{
  Attributes::addEnabled(commandStation, false);
  Attributes::addValues(commandStation, XpressNetCommandStationValues);
  m_interfaceItems.add(commandStation);
  Attributes::addEnabled(useEmergencyStopLocomotiveCommand, false);
  m_interfaceItems.add(useEmergencyStopLocomotiveCommand);
  //Attributes::addEnabled(useFunctionStateCommands, false);
  //m_interfaceItems.add(useFunctionStateCommands);
  Attributes::addEnabled(useRocoF13F20Command, false);
  m_interfaceItems.add(useRocoF13F20Command);
}

void XpressNet::worldEvent(WorldState state, WorldEvent event)
{
  SubObject::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  commandStation.setAttributeEnabled(editable);
  useEmergencyStopLocomotiveCommand.setAttributeEnabled(editable);
  //useFunctionStateCommands.setAttributeEnabled(editable);
  useRocoF13F20Command.setAttributeEnabled(editable);
}

void XpressNet::receive(const Message& message)
{
  // NOTE: this runs outside the event loop !!!

  assert(isChecksumValid(message));

  if(m_debugLog)
    EventLoop::call([this, log="rx: " + toString(message)](){ logDebug(log); });

  if(m_commandStation)
  {
    switch(message.identification())
    {
      case 0x60:
        if(message == NormalOperationResumed())
        {
          EventLoop::call(
            [cs=m_commandStation->shared_ptr<CommandStation>()]()
            {
              cs->emergencyStop.setValueInternal(false);
              cs->trackVoltageOff.setValueInternal(false);
            });
        }
        else if(message == TrackPowerOff())
        {
          EventLoop::call(
            [cs=m_commandStation->shared_ptr<CommandStation>()]()
            {
              cs->trackVoltageOff.setValueInternal(true);
            });
        }
        break;

      case 0x80:
        if(message == EmergencyStop())
        {
          EventLoop::call(
            [cs=m_commandStation->shared_ptr<CommandStation>()]()
            {
              cs->emergencyStop.setValueInternal(true);
            });
        }
        break;
    }
  }
}

void XpressNet::emergencyStopChanged(bool value)
{
  if(value)
    send(EmergencyStop());
  else if(m_commandStation && !m_commandStation->trackVoltageOff)
    send(NormalOperationResumed());
}

void XpressNet::trackVoltageOffChanged(bool value)
{
  if(!value)
    send(NormalOperationResumed());
  else
    send(TrackPowerOff());
}

void XpressNet::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  logDebug("XpressNet::decoderChanged");

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
          decoder.getFunctionValue(0)));
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
        logWarning(std::to_string(decoder.speedSteps) + " speed steps not supported");
        break;
    }
  }
  else if(has(changes, DecoderChangeFlags::FunctionValue))
  {
    if(functionNumber <= 4)
      send(FunctionInstructionGroup1(
        decoder.address,
        decoder.longAddress,
        decoder.getFunctionValue(0),
        decoder.getFunctionValue(1),
        decoder.getFunctionValue(2),
        decoder.getFunctionValue(3),
        decoder.getFunctionValue(4)));
    else if(functionNumber <= 8)
      send(FunctionInstructionGroup2(
        decoder.address,
        decoder.longAddress,
        decoder.getFunctionValue(5),
        decoder.getFunctionValue(6),
        decoder.getFunctionValue(7),
        decoder.getFunctionValue(8)));
    else if(functionNumber <= 12)
      send(FunctionInstructionGroup3(
        decoder.address,
        decoder.longAddress,
        decoder.getFunctionValue(9),
        decoder.getFunctionValue(10),
        decoder.getFunctionValue(11),
        decoder.getFunctionValue(12)));
    else if(useRocoF13F20Command && functionNumber <= 20)
      send(RocoFunctionInstructionF13F20(
        decoder.address,
        decoder.longAddress,
        decoder.getFunctionValue(13),
        decoder.getFunctionValue(14),
        decoder.getFunctionValue(15),
        decoder.getFunctionValue(16),
        decoder.getFunctionValue(17),
        decoder.getFunctionValue(18),
        decoder.getFunctionValue(19),
        decoder.getFunctionValue(20)));
    else
      logWarning("Function F" + std::to_string(functionNumber) + " not supported");
  }
}

}
