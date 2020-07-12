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
#include "../../core/eventloop.hpp"
#include "../../core/traintastic.hpp"
#include "../commandstation/commandstation.hpp"
#include "../input/loconetinput.hpp"
#include "../../utils/to_hex.hpp"

namespace Protocol {

static std::string to_string(LocoNet::OpCode value)
{
  switch(value)
  {
    case LocoNet::OPC_BUSY: return "OPC_BUSY";
    case LocoNet::OPC_GPOFF: return "OPC_GPOFFqqq";
    case LocoNet::OPC_GPON: return "OPC_GPON";
    case LocoNet::OPC_IDLE: return "OPC_IDLE";
    case LocoNet::OPC_LOCO_SPD: return "OPC_LOCO_SPD";
    case LocoNet::OPC_LOCO_DIRF: return "OPC_LOCO_DIRF";
    case LocoNet::OPC_LOCO_SND: return "OPC_LOCO_SND";
    case LocoNet::OPC_SW_REQ: return "OPC_SW_REQ";
    case LocoNet::OPC_SW_REP: return "OPC_SW_REP";
    case LocoNet::OPC_INPUT_REP: return "OPC_INPUT_REP";
    case LocoNet::OPC_LONG_ACK: return "OPC_LONG_ACK";
    case LocoNet::OPC_SLOT_STAT1: return "OPC_SLOT_STAT1";
    case LocoNet::OPC_CONSIST_FUNC: return "OPC_CONSIST_FUNC";
    case LocoNet::OPC_UNLINK_SLOTS: return "OPC_UNLINK_SLOTS";
    case LocoNet::OPC_LINK_SLOTS: return "OPC_LINK_SLOTS";
    case LocoNet::OPC_MOVE_SLOTS: return "OPC_MOVE_SLOTS";
    case LocoNet::OPC_RQ_SL_DATA: return "OPC_RQ_SL_DATA";
    case LocoNet::OPC_SW_STATE: return "OPC_SW_STATE";
    case LocoNet::OPC_SW_ACK: return "OPC_SW_ACK";
    case LocoNet::OPC_LOCO_ADR: return "OPC_LOCO_ADR";
    case LocoNet::OPC_MULTI_SENSE: return "OPC_MULTI_SENSE";
    case LocoNet::OPC_PEER_XFER: return "OPC_PEER_XFER";
    case LocoNet::OPC_SL_RD_DATA: return "OPC_SL_RD_DATA";
    case LocoNet::OPC_IMM_PACKET: return "OPC_IMM_PACKET";
    case LocoNet::OPC_WR_SL_DATA: return "OPC_WR_SL_DATA";
  }

  return to_hex(value);
}

std::string to_string(const LocoNet::Message& message, bool raw = false)
{
  std::string s{to_string(message.opCode)};

  switch(message.opCode)
  {
    case LocoNet::OPC_GPON:
    case LocoNet::OPC_GPOFF:
    case LocoNet::OPC_IDLE:
    case LocoNet::OPC_BUSY:
      break;

    case LocoNet::OPC_LOCO_SPD:
    {
      const LocoNet::LocoSpd& locoSpd = static_cast<const LocoNet::LocoSpd&>(message);
      s.append(" slot=").append(std::to_string(locoSpd.slot));
      s.append(" speed=").append(std::to_string(locoSpd.speed));
      break;
    }
    case LocoNet::OPC_LOCO_DIRF:
    {
      const LocoNet::LocoDirF& locoDirF = static_cast<const LocoNet::LocoDirF&>(message);
      s.append(" slot=").append(std::to_string(locoDirF.slot));
      s.append(" dir=").append(locoDirF.direction() == Direction::Forward ? "fwd" : "rev");
      s.append(" f0=").append(locoDirF.f0() ? "on" : "off");
      s.append(" f1=").append(locoDirF.f1() ? "on" : "off");
      s.append(" f2=").append(locoDirF.f2() ? "on" : "off");
      s.append(" f3=").append(locoDirF.f3() ? "on" : "off");
      s.append(" f4=").append(locoDirF.f4() ? "on" : "off");
      break;
    }
    case LocoNet::OPC_LOCO_SND:
    {
      const LocoNet::LocoSnd& locoSnd = static_cast<const LocoNet::LocoSnd&>(message);
      s.append(" slot=").append(std::to_string(locoSnd.slot));
      s.append(" f5=").append(locoSnd.f5() ? "on" : "off");
      s.append(" f6=").append(locoSnd.f6() ? "on" : "off");
      s.append(" f7=").append(locoSnd.f7() ? "on" : "off");
      s.append(" f8=").append(locoSnd.f8() ? "on" : "off");
      break;
    }
    case LocoNet::OPC_INPUT_REP:
    {
      const LocoNet::InputRep& inputRep = static_cast<const LocoNet::InputRep&>(message);
      s.append(" address=").append(std::to_string(inputRep.address()));
      s.append(" input=").append(inputRep.isAuxInput() ? "aux" : "switch");
      s.append(" value=").append(inputRep.value() ? "high" : "low");
      break;
    }
    case LocoNet::OPC_RQ_SL_DATA:
    {
      const LocoNet::RequestSlotData& requestSlotData = static_cast<const LocoNet::RequestSlotData&>(message);
      s.append(" slot=").append(std::to_string(requestSlotData.slot));
      break;
    }
    case LocoNet::OPC_MULTI_SENSE:
    {
      const LocoNet::MultiSense& multiSense = static_cast<const LocoNet::MultiSense&>(message);
      if(multiSense.isTransponder())
      {
        const LocoNet::MultiSenseTransponder& multiSenseTransponder = static_cast<const LocoNet::MultiSenseTransponder&>(multiSense);
        s.append(multiSenseTransponder.isPresent() ? " present" : " absent");
        s.append(" sensorAddress=").append(std::to_string(multiSenseTransponder.sensorAddress()));
        s.append(" transponderAddress=").append(std::to_string(multiSenseTransponder.transponderAddress()));
      }
      else
        raw = true;
      break;
    }
    case LocoNet::OPC_MULTI_SENSE_LONG:
    {
      const LocoNet::MultiSenseLong& multiSense = static_cast<const LocoNet::MultiSenseLong&>(message);
      if(multiSense.isTransponder())
      {
        const LocoNet::MultiSenseLongTransponder& multiSenseTransponder = static_cast<const LocoNet::MultiSenseLongTransponder&>(multiSense);
        s.append(multiSenseTransponder.isPresent() ? " present" : " absent");
        s.append(" sensorAddress=").append(std::to_string(multiSenseTransponder.sensorAddress()));
        s.append(" transponderAddress=").append(std::to_string(multiSenseTransponder.transponderAddress()));
        s.append(" transponderDirection=").append(multiSenseTransponder.transponderDirection() == Direction::Forward ? "fwd" : "rev");
      }
      else
        raw = true;
      break;
    }
    default:
      raw = true;
      break;
  }

  if(raw)
  {
    s.append(" [");
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&message);
    for(int i = 0; i < message.size(); i++)
    {
      if(i != 0)
        s.append(" ");
      s.append(to_hex(bytes[i]));
    }
    s.append("]");
  }

  return s;
}

constexpr bool isLongAddress(uint16_t address)
{
  return address > 127;
}

void updateDecoderSpeed(const std::shared_ptr<Hardware::Decoder>& decoder, uint8_t speed)
{
  decoder->emergencyStop.setValueInternal(speed == LocoNet::SPEED_ESTOP);

  if(speed == LocoNet::SPEED_STOP || speed == LocoNet::SPEED_ESTOP)
    decoder->speedStep.setValueInternal(0);
  else
    decoder->speedStep.setValueInternal(((speed - 1) * decoder->speedSteps) / (LocoNet::SPEED_MAX - 1));
}

uint8_t LocoNet::calcChecksum(const Message& msg)
{
  const uint8_t* p = reinterpret_cast<const uint8_t*>(&msg);
  const int size = msg.size() - 1;
  uint8_t checksum = 0xFF;
  for(int i = 0; i < size; i++)
    checksum ^= p[i];
  return checksum;
}

void LocoNet::updateChecksum(Message& msg)
{
  reinterpret_cast<uint8_t*>(&msg)[msg.size() - 1] = calcChecksum(msg);
}

bool LocoNet::isChecksumValid(const Message& msg)
{
  return calcChecksum(msg) == reinterpret_cast<const uint8_t*>(&msg)[msg.size() - 1];
}

LocoNet::LocoNet(Object& _parent, const std::string& parentPropertyName, std::function<bool(const Message&)> send) :
  SubObject(_parent, parentPropertyName),
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

void LocoNet::send(uint16_t address, SlotMessage& message)
{
  if((message.slot = m_slots.getSlot(address)) != SLOT_UNKNOWN)
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
          if(auto cs = std::dynamic_pointer_cast<Hardware::CommandStation::CommandStation>(parent().shared_from_this()))
          {
            cs->emergencyStop.setValueInternal(false);
            cs->trackVoltageOff.setValueInternal(false);
          }
        });
      break;

    case OPC_GPOFF:
      EventLoop::call(
        [this]()
        {
          if(auto cs = std::dynamic_pointer_cast<Hardware::CommandStation::CommandStation>(parent().shared_from_this()))
            cs->trackVoltageOff.setValueInternal(true);
        });
      break;

    case OPC_IDLE:
      EventLoop::call(
        [this]()
        {
          if(auto cs = std::dynamic_pointer_cast<Hardware::CommandStation::CommandStation>(parent().shared_from_this()))
            cs->emergencyStop.setValueInternal(true);
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

void LocoNet::decoderChanged(const Hardware::Decoder& decoder, Hardware::DecoderChangeFlags changes, uint32_t functionNumber)
{
  using namespace Hardware;

  logDebug("LocoNet::decoderChanged");

  if(has(changes, DecoderChangeFlags::EmergencyStop | DecoderChangeFlags::SpeedStep))
  {
    LocoSpd message{static_cast<uint8_t>(decoder.emergencyStop ? 1 : (decoder.speedStep > 0 ? 1 + decoder.speedStep : 0))};
    send(decoder.address, message);
  }
  else if(has(changes, DecoderChangeFlags::FunctionValue | DecoderChangeFlags::Direction))
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

  if(auto cs = std::dynamic_pointer_cast<Hardware::CommandStation::CommandStation>(parent().shared_from_this()))
  {
    const uint16_t address = m_slots.getAddress(slot);
    if(address != 0)
    {
      auto decoder = cs->getDecoder(DecoderProtocol::DCC, address, isLongAddress(address));
      if(!decoder)
        decoder = cs->getDecoder(DecoderProtocol::Auto, address);
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
