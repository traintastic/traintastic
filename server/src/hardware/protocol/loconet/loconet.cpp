/**
 * server/src/hardware/protocol/loconet.cpp
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

#include "loconet.hpp"
#include <thread>
#include <chrono>
#include "../../../core/eventloop.hpp"
#include "../../../core/traintastic.hpp"
#include "../../commandstation/commandstation.hpp"
#include "../../input/loconetinput.hpp"

namespace Protocol::LocoNet {

void updateDecoderSpeed(const std::shared_ptr<Hardware::Decoder>& decoder, uint8_t speed)
{
  decoder->emergencyStop.setValueInternal(speed == SPEED_ESTOP);

  if(speed == SPEED_STOP || speed == SPEED_ESTOP)
    decoder->speedStep.setValueInternal(0);
  else
    decoder->speedStep.setValueInternal(((speed - 1) * decoder->speedSteps) / (SPEED_MAX - 1));
}

LocoNet::LocoNet(Object& _parent, const std::string& parentPropertyName, std::function<bool(const Message&)> send) :
  SubObject(_parent, parentPropertyName),
  m_commandStation{dynamic_cast<Hardware::CommandStation::CommandStation*>(&_parent)},
  m_send{std::move(send)},
  m_debugLog{true/*false*/},
  m_queryLocoSlots{SLOT_UNKNOWN},
  commandStation{this, "command_station", LocoNetCommandStation::Custom, PropertyFlags::ReadWrite,
    [this](LocoNetCommandStation value)
    {
      switch(value)
      {
        case LocoNetCommandStation::Custom:
          break;

        case LocoNetCommandStation::DigiKeijsDR5000:
          break;

        case LocoNetCommandStation::UhlenbrockIntellibox:
          break;
      }
    }},
  debugLog{this, "debug_log", m_debugLog, PropertyFlags::ReadWrite | PropertyFlags::Store,
   [this](bool value)
    {
      m_debugLog = value;
    }}
{
  assert(m_send);

  m_interfaceItems.add(commandStation)
    .addAttributeEnabled(false);
  m_interfaceItems.add(debugLog);
}

bool LocoNet::send(const Message& message)
{
  if(m_debugLog)
    logDebug("tx: " + to_string(message));
  assert(isChecksumValid(message));
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

  assert(isChecksumValid(message));

  if(m_debugLog)
    EventLoop::call([this, log="rx: " + to_string(message)](){ logDebug(log); });

  switch(message.opCode)
  {
    case OPC_GPON:
      EventLoop::call(
        [this]()
        {
          if(m_commandStation)
          {
            m_commandStation->emergencyStop.setValueInternal(false);
            m_commandStation->trackVoltageOff.setValueInternal(false);
          }
        });
      break;

    case OPC_GPOFF:
      EventLoop::call(
        [this]()
        {
          if(m_commandStation)
            m_commandStation->trackVoltageOff.setValueInternal(true);
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
          auto it = m_inputs.find(1 + inputRep.address());
          if(it != m_inputs.end())
            it->second->valueChanged(inputRep.value());
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
  else if(m_commandStation && !m_commandStation->trackVoltageOff)
    send(GlobalPowerOn());
}

void LocoNet::trackVoltageOffChanged(bool value)
{
  if(!value)
    send(GlobalPowerOn());
  else
    send(GlobalPowerOff());
}

void LocoNet::decoderChanged(const Hardware::Decoder& decoder, Hardware::DecoderChangeFlags changes, uint32_t functionNumber)
{
  using namespace Hardware;

  logDebug("LocoNet::decoderChanged");

  if(has(changes, DecoderChangeFlags::EmergencyStop | DecoderChangeFlags::SpeedStep))
  {
    LocoSpd message{static_cast<uint8_t>(decoder.emergencyStop ? 1 : (decoder.speedStep > 0 ? 1 + decoder.speedStep : 0))};
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

std::shared_ptr<Hardware::Decoder> LocoNet::getDecoder(uint8_t slot, bool request)
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

bool LocoNet::isInputAddressAvailable(uint16_t address)
{
  return m_inputs.find(address) == m_inputs.end();
}

bool LocoNet::addInput(const std::shared_ptr<LocoNetInput>& input)
{
  assert(input->loconet.value().get() == this);
  if(isInputAddressAvailable(input->address))
  {
    m_inputs.insert({input->address, input});
    return true;
  }
  else
    return false;
}

void LocoNet::removeInput(const std::shared_ptr<LocoNetInput>& input)
{
  assert(input->loconet.value().get() == this);
  m_inputs.erase(m_inputs.find(input->address));
}

}
