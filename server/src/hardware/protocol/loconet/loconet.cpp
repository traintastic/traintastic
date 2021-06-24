/**
 * server/src/hardware/protocol/loconet.cpp
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

#include "loconet.hpp"
#include <thread>
#include <chrono>
#include "../../../core/eventloop.hpp"
#include "../../../core/traintastic.hpp"
#include "../../commandstation/commandstation.hpp"
#include "../../input/loconetinput.hpp"
#include "../../output/loconetoutput.hpp"
#include "../../../core/attributes.hpp"
#include "../../../world/getworld.hpp"
#include "loconetlisttablemodel.hpp"

namespace LocoNet {

void updateDecoderSpeed(const std::shared_ptr<Decoder>& decoder, uint8_t speed)
{
  decoder->emergencyStop.setValueInternal(speed == SPEED_ESTOP);

  if(speed == SPEED_STOP || speed == SPEED_ESTOP)
    decoder->speedStep.setValueInternal(0);
  else
    decoder->speedStep.setValueInternal(((speed - 1) * decoder->speedSteps) / (SPEED_MAX - 1));
}

std::shared_ptr<LocoNet> LocoNet::create(Object& _parent, const std::string& parentPropertyName, std::function<bool(const Message&)> send)
{
  std::shared_ptr<LocoNet> object{std::make_shared<LocoNet>(_parent, parentPropertyName, std::move(send), Private())};
  if(auto w = getWorld(&_parent))
    w->loconets->addObject(object);
  return object;
}

LocoNet::LocoNet(Object& _parent, const std::string& parentPropertyName, std::function<bool(const Message&)> send, Private) :
  SubObject(_parent, parentPropertyName),
  m_commandStation{dynamic_cast<CommandStation*>(&_parent)},
  m_send{std::move(send)},
  m_debugLogRXTX{false},
  m_queryLocoSlots{SLOT_UNKNOWN},
  commandStation{this, "command_station", LocoNetCommandStation::Custom, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](LocoNetCommandStation value)
    {
      switch(value)
      {
        case LocoNetCommandStation::Custom:
          break;

        case LocoNetCommandStation::DigikeijsDR5000:
          break;

        case LocoNetCommandStation::UhlenbrockIntellibox:
          break;
      }
    }},
  debugLogInput{this, "debug_log_input", false, PropertyFlags::ReadWrite | PropertyFlags::Store},
  debugLogOutput{this, "debug_log_output", false, PropertyFlags::ReadWrite | PropertyFlags::Store},
  debugLogRXTX{this, "debug_log_rx_tx", m_debugLogRXTX, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](bool value)
    {
      m_debugLogRXTX = value;
    }},
  inputMonitor{*this, "input_monitor",
    [this]()
    {
      return std::make_shared<LocoNetInputMonitor>(shared_ptr<LocoNet>());
    }},
  outputKeyboard{*this, "output_keyboard",
    [this]()
    {
      return std::make_shared<LocoNetOutputKeyboard>(shared_ptr<LocoNet>());
    }}
{
  assert(m_send);

  Attributes::addEnabled(commandStation, m_commandStation && !m_commandStation->online);
  Attributes::addValues(commandStation, LocoNetCommandStationValues);
  m_interfaceItems.add(commandStation);
  //Attributes::addGroup(debugLogInput);
  m_interfaceItems.add(debugLogInput);
  //Attributes::addGroup(debugLogOuput);
  m_interfaceItems.add(debugLogOutput);
  //Attributes::addGroup(debugLogRXTX);
  m_interfaceItems.add(debugLogRXTX);
  m_interfaceItems.add(inputMonitor);
  m_interfaceItems.add(outputKeyboard);
}

bool LocoNet::send(const Message& message)
{
  if(m_debugLogRXTX)
    logDebug("tx: " + toString(message));
  assert(isValid(message));
  return m_send(message);
}

void LocoNet::send(uint16_t address, Message& message, uint8_t& slot)
{
  if((slot = m_slots.getSlot(address)) != SLOT_UNKNOWN)
  {
    updateChecksum(message);
    send(message);
  }
  else // try get a slot
  {
    std::byte* ptr = reinterpret_cast<std::byte*>(&message);

    auto it = m_slotRequests.find(address);
    if(it == m_slotRequests.end())
    {
      m_slotRequests[address].assign(ptr, ptr + message.size());
      send(LocoAdr{address});
    }
    else
       it->second.insert(it->second.end(), ptr, ptr + message.size());
  }
}

void LocoNet::receive(const Message& message)
{
  // NOTE: this function is called async!

  assert(isValid(message));

  if(m_debugLogRXTX)
    EventLoop::call([this, log="rx: " + toString(message)](){ logDebug(log); });

  switch(message.opCode)
  {
    case OPC_GPON:
      EventLoop::call(
        [this]()
        {
          if(m_commandStation)
          {
            m_commandStation->emergencyStop.setValueInternal(false);
            m_commandStation->powerOn.setValueInternal(true);
          }
        });
      break;

    case OPC_GPOFF:
      EventLoop::call(
        [this]()
        {
          if(m_commandStation)
            m_commandStation->powerOn.setValueInternal(false);
        });
      break;

    case OPC_IDLE:
      EventLoop::call(
        [this]()
        {
          if(m_commandStation)
            m_commandStation->emergencyStop.setValueInternal(true);
        });
      break;

    case OPC_LOCO_SPD:
      EventLoop::call(
        [this, locoSpd=*static_cast<const LocoSpd*>(&message)]()
        {
          if(auto decoder = getDecoder(locoSpd.slot))
            updateDecoderSpeed(decoder, locoSpd.speed);
        });
      break;

    case OPC_LOCO_DIRF:
      EventLoop::call(
        [this, locoDirF=*static_cast<const LocoDirF*>(&message)]()
        {
          if(auto decoder = getDecoder(locoDirF.slot))
          {
            decoder->direction.setValueInternal(locoDirF.direction());
            decoder->setFunctionValue(0, locoDirF.f0());
            decoder->setFunctionValue(1, locoDirF.f1());
            decoder->setFunctionValue(2, locoDirF.f2());
            decoder->setFunctionValue(3, locoDirF.f3());
            decoder->setFunctionValue(4, locoDirF.f4());
          }
        });
      break;

    case OPC_LOCO_SND:
      EventLoop::call(
        [this, locoSnd=*static_cast<const LocoSnd*>(&message)]()
        {
          if(auto decoder = getDecoder(locoSnd.slot))
          {
            decoder->setFunctionValue(5, locoSnd.f5());
            decoder->setFunctionValue(6, locoSnd.f6());
            decoder->setFunctionValue(7, locoSnd.f7());
            decoder->setFunctionValue(8, locoSnd.f8());
          }
        });
      break;

    case OPC_LOCO_F9F12:
      EventLoop::call(
        [this, locoF9F12=*static_cast<const LocoF9F12*>(&message)]()
        {
          if(auto decoder = getDecoder(locoF9F12.slot))
          {
            decoder->setFunctionValue(9, locoF9F12.f9());
            decoder->setFunctionValue(10, locoF9F12.f10());
            decoder->setFunctionValue(11, locoF9F12.f11());
            decoder->setFunctionValue(12, locoF9F12.f12());
          }
        });
      break;

    case OPC_INPUT_REP:
      EventLoop::call(
        [this, inputRep=*static_cast<const InputRep*>(&message)]()
        {
          const uint16_t address = 1 + inputRep.fullAddress();

          if(debugLogInput)
            logDebug(std::string("input ").append(std::to_string(address)).append(" = ").append(inputRep.value() ? "1" : "0"));

          auto it = m_inputs.find(address);
          if(it != m_inputs.end())
            it->second->updateValue(toTriState(inputRep.value()));
          else
            inputMonitorValueChanged(address, toTriState(inputRep.value()));
        });
      break;

    case OPC_SW_REQ:
      EventLoop::call(
        [this, switchRequest=*static_cast<const SwitchRequest*>(&message)]()
        {
          const uint16_t address = 1 + switchRequest.fullAddress();

          if(debugLogOutput)
            logDebug(std::string("output ").append(std::to_string(address)).append(" = ").append(switchRequest.on() ? "on" : "off"));

          auto it = m_outputs.find(address);
          if(it != m_outputs.end())
            it->second->updateValue(toTriState(switchRequest.on()));
          else
            outputKeyboardValueChanged(address, toTriState(switchRequest.on()));
        });
      break;

    case OPC_SL_RD_DATA:
      EventLoop::call(
        [this, slotReadData=*static_cast<const SlotReadData*>(&message)]()
        {
          if(m_queryLocoSlots == slotReadData.slot)
          {
            m_queryLocoSlots++;
            if(m_queryLocoSlots <= SLOT_LOCO_MAX)
              send(RequestSlotData(m_queryLocoSlots));
            else
              m_queryLocoSlots = SLOT_UNKNOWN; // done
          }

          if(slotReadData.isBusy() || slotReadData.isActive())
          {
            m_slots.set(slotReadData.address(), slotReadData.slot);

            logDebug("slot " + std::to_string(slotReadData.slot) + " = " + std::to_string(slotReadData.address()));

            if(auto decoder = getDecoder(slotReadData.slot, false))
            {
              updateDecoderSpeed(decoder, slotReadData.spd);
              decoder->direction.setValueInternal(slotReadData.direction());
              decoder->setFunctionValue(0, slotReadData.f0());
              decoder->setFunctionValue(1, slotReadData.f1());
              decoder->setFunctionValue(2, slotReadData.f2());
              decoder->setFunctionValue(3, slotReadData.f3());
              decoder->setFunctionValue(4, slotReadData.f4());
              decoder->setFunctionValue(5, slotReadData.f5());
              decoder->setFunctionValue(6, slotReadData.f6());
              decoder->setFunctionValue(7, slotReadData.f7());
              decoder->setFunctionValue(8, slotReadData.f8());
            }
          }
          else
            logDebug("slot " + std::to_string(slotReadData.slot) + " = FREE");
        });
      break;
  }
}

void LocoNet::emergencyStopChanged(bool value)
{
  if(value)
    send(Idle());
  else if(m_commandStation && m_commandStation->powerOn)
    send(GlobalPowerOn());
}

void LocoNet::powerOnChanged(bool value)
{
  if(value)
    send(GlobalPowerOn());
  else
    send(GlobalPowerOff());
}

void LocoNet::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(has(changes, DecoderChangeFlags::EmergencyStop | DecoderChangeFlags::SpeedStep))
  {
    const bool emergencyStop = decoder.emergencyStop || (m_commandStation && m_commandStation->emergencyStop);
    LocoSpd message{static_cast<uint8_t>(emergencyStop ? 1 : (decoder.speedStep > 0 ? 1 + decoder.speedStep : 0))};
    send(decoder.address, message);
  }

  if(has(changes, DecoderChangeFlags::FunctionValue | DecoderChangeFlags::Direction))
  {
    if(functionNumber <= 4 || has(changes, DecoderChangeFlags::Direction))
    {
      LocoDirF message{
        decoder.direction,
        decoder.getFunctionValue(0),
        decoder.getFunctionValue(1),
        decoder.getFunctionValue(2),
        decoder.getFunctionValue(3),
        decoder.getFunctionValue(4)};
      send(decoder.address, message);
    }
    else if(functionNumber <= 8)
    {
      LocoSnd message{
        decoder.getFunctionValue(5),
        decoder.getFunctionValue(6),
        decoder.getFunctionValue(7),
        decoder.getFunctionValue(8)};
      send(decoder.address, message);
    }
    else if(functionNumber <= 12)
    {
      LocoF9F12 message{
        decoder.getFunctionValue(9),
        decoder.getFunctionValue(10),
        decoder.getFunctionValue(11),
        decoder.getFunctionValue(12)};
      send(decoder.address, message);
    }
    else if(functionNumber <= 19)
    {
      LocoF13F19 message{
        decoder.getFunctionValue(13),
        decoder.getFunctionValue(14),
        decoder.getFunctionValue(15),
        decoder.getFunctionValue(16),
        decoder.getFunctionValue(17),
        decoder.getFunctionValue(18),
        decoder.getFunctionValue(19)};
      send(decoder.address, message);
    }
    else if(functionNumber == 20 || functionNumber == 28)
    {
      LocoF20F28 message{
        decoder.getFunctionValue(20),
        decoder.getFunctionValue(28)};
      send(decoder.address, message);
    }
    else if(functionNumber <= 27)
    {
      LocoF21F27 message{
        decoder.getFunctionValue(21),
        decoder.getFunctionValue(22),
        decoder.getFunctionValue(23),
        decoder.getFunctionValue(24),
        decoder.getFunctionValue(25),
        decoder.getFunctionValue(26),
        decoder.getFunctionValue(27)};
      send(decoder.address, message);
    }
    else
      logWarning("Function F" + std::to_string(functionNumber) + " not supported");
  }
}

void LocoNet::queryLocoSlots()
{
  m_queryLocoSlots = SLOT_LOCO_MIN;
  send(RequestSlotData(m_queryLocoSlots));
}

uint16_t LocoNet::getUnusedInputAddress() const
{
  const auto end = m_inputs.cend();
  for(uint16_t address = LocoNetInput::addressMin; address < LocoNetInput::addressMax; address++)
    if(m_inputs.find(address) == end)
      return address;
  return LocoNetInput::addressInvalid;
}

uint16_t LocoNet::getUnusedOutputAddress() const
{
  const auto end = m_outputs.cend();
  for(uint16_t address = LocoNetOutput::addressMin; address < LocoNetOutput::addressMax; address++)
    if(m_outputs.find(address) == end)
      return address;
  return LocoNetOutput::addressInvalid;
}

std::shared_ptr<Decoder> LocoNet::getDecoder(uint8_t slot, bool request)
{
  if(slot < SLOT_LOCO_MIN || slot > SLOT_LOCO_MAX)
    return nullptr;

  if(m_commandStation)
  {
    const uint16_t address = m_slots.getAddress(slot);
    if(address != 0)
    {
      auto decoder = m_commandStation->getDecoder(DecoderProtocol::DCC, address, isLongAddress(address));
      if(!decoder)
        decoder = m_commandStation->getDecoder(DecoderProtocol::Auto, address);
      return decoder;
    }
    else if(request)
      send(RequestSlotData(slot));
  }

  return nullptr;
}

bool LocoNet::isInputAddressAvailable(uint16_t address) const
{
  return m_inputs.find(address) == m_inputs.end();
}

bool LocoNet::changeInputAddress(LocoNetInput& input, uint16_t newAddress)
{
  assert(input.loconet.value().get() == this);

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

bool LocoNet::addInput(LocoNetInput& input)
{
  if(isInputAddressAvailable(input.address))
  {
    m_inputs.insert({input.address, input.shared_ptr<LocoNetInput>()});
    inputMonitorIdChanged(input.address, input.id.value());
    input.updateValue(TriState::Undefined);
    // TODO: request state!
    return true;
  }
  else
    return false;
}

void LocoNet::removeInput(LocoNetInput& input)
{
  assert(input.loconet.value().get() == this);
  const uint16_t address = input.address;
  auto it = m_inputs.find(input.address);
  if(it != m_inputs.end() && it->second.get() == &input)
    m_inputs.erase(it);
  input.updateValue(TriState::Undefined);
  inputMonitorIdChanged(address, {});
}

void LocoNet::inputMonitorIdChanged(const uint32_t address, std::string_view value)
{
  for(auto* inputMonitor : m_inputMonitors)
    if(inputMonitor->inputIdChanged)
      inputMonitor->inputIdChanged(*inputMonitor, address, value);
}

void LocoNet::inputMonitorValueChanged(const uint32_t address, const TriState value)
{
  for(auto* inputMonitor : m_inputMonitors)
    if(inputMonitor->inputValueChanged)
      inputMonitor->inputValueChanged(*inputMonitor, address, value);
}

bool LocoNet::isOutputAddressAvailable(uint16_t address) const
{
  return m_outputs.find(address) == m_outputs.end();
}

bool LocoNet::changeOutputAddress(LocoNetOutput& output, uint16_t newAddress)
{
  assert(output.loconet.value().get() == this);

  if(!isOutputAddressAvailable(newAddress))
    return false;

  auto node = m_outputs.extract(output.address); // old address
  node.key() = newAddress;
  m_outputs.insert(std::move(node));
  outputKeyboardIdChanged(output.address, {});
  outputKeyboardIdChanged(newAddress, output.id.value());
  output.value.setValueInternal(TriState::Undefined);
  // TODO: request state!

  return true;
}

bool LocoNet::addOutput(LocoNetOutput& output)
{
  if(isOutputAddressAvailable(output.address))
  {
    m_outputs.insert({output.address, output.shared_ptr<LocoNetOutput>()});
    outputKeyboardIdChanged(output.address, output.id.value());
    output.value.setValueInternal(TriState::Undefined);
    return true;
  }
  else
    return false;
}

void LocoNet::removeOutput(LocoNetOutput& output)
{
  assert(output.loconet.value().get() == this);
  const uint16_t address = output.address;
  auto it = m_outputs.find(output.address);
  if(it != m_outputs.end() && it->second.get() == &output)
    m_outputs.erase(it);
  output.value.setValueInternal(TriState::Undefined);
  outputKeyboardIdChanged(address, {});
}

void LocoNet::outputKeyboardIdChanged(const uint32_t address, std::string_view value)
{
  for(auto* outputKeyboard : m_outputKeyboards)
    if(outputKeyboard->outputIdChanged)
      outputKeyboard->outputIdChanged(*outputKeyboard, address, value);
}

void LocoNet::outputKeyboardValueChanged(const uint32_t address, const TriState value)
{
  for(auto* outputKeyboard : m_outputKeyboards)
    if(outputKeyboard->outputValueChanged)
      outputKeyboard->outputValueChanged(*outputKeyboard, address, value);
}

}
