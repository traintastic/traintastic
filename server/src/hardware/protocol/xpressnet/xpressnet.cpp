/**
 * server/src/hardware/protocol/xpressnet/xpressnet.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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
#include "../../input/xpressnetinput.hpp"
#include "../../../log/log.hpp"
#include "../../../utils/displayname.hpp"

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
    }},
  inputMonitor{*this, "input_monitor",
    [this]()
    {
      return std::make_shared<XpressNetInputMonitor>(shared_ptr<XpressNet>());
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
  m_interfaceItems.add(debugLog);

  Attributes::addDisplayName(inputMonitor, DisplayName::Hardware::inputMonitor);
  m_interfaceItems.add(inputMonitor);
}

void XpressNet::worldEvent(WorldState state, WorldEvent event)
{
  SubObject::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(commandStation, editable);
  Attributes::setEnabled(useEmergencyStopLocomotiveCommand, editable);
  //Attributes::setEnabled(useFunctionStateCommands, editable);
  Attributes::setEnabled(useRocoF13F20Command, editable);
}

void XpressNet::receive(const Message& message)
{
  // NOTE: this runs outside the event loop !!!

  assert(isChecksumValid(message));

  if(m_debugLog)
    EventLoop::call([this, data=toString(message)](){ Log::log(*this, LogMessage::D2002_RX_X, data); });

  if(m_commandStation)
  {
    switch(message.identification())
    {
      case idFeedbackBroadcast:
      {
        const FeedbackBroadcast* fb = static_cast<const FeedbackBroadcast*>(&message);

        for(uint8_t i = 0; i < fb->pairCount(); i++)
        {
          const FeedbackBroadcast::Pair& p = fb->pair(i);
          if(p.type() == FeedbackBroadcast::Pair::Type::FeedbackModule)
          {
            const uint16_t baseAddress = 1 + (static_cast<uint16_t>(p.address) << 3) + (p.isHighNibble() ? 4 : 0);
            EventLoop::call(
              [this, baseAddress, status=p.statusNibble()]()
              {
                for(uint16_t j = 0; j < 4; j++)
                {
                  TriState v = toTriState((status & (1 << j)) != 0);
                  auto it = m_inputs.find(baseAddress + j);
                  if(it != m_inputs.end())
                    it->second->updateValue(v);
                  else
                    inputMonitorValueChanged(baseAddress + j, v);
                }
              });
          }
        }
        break;
      }
      case 0x60:
        if(message == NormalOperationResumed())
        {
          EventLoop::call(
            [cs=m_commandStation->shared_ptr<CommandStation>()]()
            {
              cs->emergencyStop.setValueInternal(false);
              cs->powerOn.setValueInternal(true);
            });
        }
        else if(message == TrackPowerOff())
        {
          EventLoop::call(
            [cs=m_commandStation->shared_ptr<CommandStation>()]()
            {
              cs->powerOn.setValueInternal(false);
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
  else if(m_commandStation && m_commandStation->powerOn)
    send(NormalOperationResumed());
}

void XpressNet::powerOnChanged(bool value)
{
  if(value)
    send(NormalOperationResumed());
  else
    send(TrackPowerOff());
}

void XpressNet::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(useEmergencyStopLocomotiveCommand && changes == DecoderChangeFlags::EmergencyStop && decoder.emergencyStop)
    send(EmergencyStopLocomotive(
          decoder.address,
          decoder.longAddress));
  else if(has(changes, DecoderChangeFlags::EmergencyStop | DecoderChangeFlags::Direction | DecoderChangeFlags::Throttle | DecoderChangeFlags::SpeedSteps))
  {
    switch(decoder.speedSteps)
    {
      case 14:
        send(SpeedAndDirectionInstruction14(
          decoder.address,
          decoder.longAddress,
          decoder.emergencyStop,
          decoder.direction,
          Decoder::throttleToSpeedStep(decoder.throttle, 14),
          decoder.getFunctionValue(0)));
        break;

      case 27:
        send(SpeedAndDirectionInstruction27(
          decoder.address,
          decoder.longAddress,
          decoder.emergencyStop,
          decoder.direction,
          Decoder::throttleToSpeedStep(decoder.throttle, 27)));
        break;

      case 28:
        send(SpeedAndDirectionInstruction28(
          decoder.address,
          decoder.longAddress,
          decoder.emergencyStop,
          decoder.direction,
          Decoder::throttleToSpeedStep(decoder.throttle, 28)));
        break;

      case 126:
      case 128:
      default:
        send(SpeedAndDirectionInstruction128(
          decoder.address,
          decoder.longAddress,
          decoder.emergencyStop,
          decoder.direction,
          Decoder::throttleToSpeedStep(decoder.throttle, 126)));
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
    else if(functionNumber <= 20)
    {
      if(useRocoF13F20Command)
        send(RocoMultiMAUS::FunctionInstructionF13F20(
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
        send(FunctionInstructionGroup4(
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
    }
    else if(functionNumber <= 28)
      send(FunctionInstructionGroup5(
        decoder.address,
        decoder.longAddress,
        decoder.getFunctionValue(21),
        decoder.getFunctionValue(22),
        decoder.getFunctionValue(23),
        decoder.getFunctionValue(24),
        decoder.getFunctionValue(25),
        decoder.getFunctionValue(26),
        decoder.getFunctionValue(27),
        decoder.getFunctionValue(28)));
  }
}

bool XpressNet::isInputAddressAvailable(uint16_t address) const
{
  return m_inputs.find(address) == m_inputs.end();
}

uint16_t XpressNet::getUnusedInputAddress() const
{
  const auto end = m_inputs.cend();
  for(uint16_t address = XpressNetInput::addressMin; address < XpressNetInput::addressMax; address++)
    if(m_inputs.find(address) == end)
      return address;
  return XpressNetInput::addressInvalid;
}

bool XpressNet::changeInputAddress(XpressNetInput& input, uint16_t newAddress)
{
  assert(input.xpressnet.value().get() == this);

  if(!isInputAddressAvailable(newAddress))
    return false;

  auto node = m_inputs.extract(input.address); // old address
  node.key() = newAddress;
  m_inputs.insert(std::move(node));
  inputMonitorIdChanged(input.address, {});
  inputMonitorIdChanged(newAddress, input.id.value());
  input.updateValue(TriState::Undefined);

  return true;
}

bool XpressNet::addInput(XpressNetInput& input)
{
  if(isInputAddressAvailable(input.address))
  {
    m_inputs.insert({input.address, input.shared_ptr<XpressNetInput>()});
    inputMonitorIdChanged(input.address, input.id.value());
    input.updateValue(TriState::Undefined);
    // TODO: request state!
    return true;
  }
  else
    return false;
}

void XpressNet::removeInput(XpressNetInput& input)
{
  assert(input.xpressnet.value().get() == this);
  const uint16_t address = input.address;
  auto it = m_inputs.find(input.address);
  if(it != m_inputs.end() && it->second.get() == &input)
    m_inputs.erase(it);
  input.updateValue(TriState::Undefined);
  inputMonitorIdChanged(address, {});
}

void XpressNet::inputMonitorIdChanged(const uint32_t address, std::string_view value)
{
  for(auto* item : m_inputMonitors)
    if(item->inputIdChanged)
      item->inputIdChanged(*item, address, value);
}

void XpressNet::inputMonitorValueChanged(const uint32_t address, const TriState value)
{
  for(auto* item : m_inputMonitors)
    if(item->inputValueChanged)
      item->inputValueChanged(*item, address, value);
}

}
