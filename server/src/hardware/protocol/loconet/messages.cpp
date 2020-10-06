/**
 * server/src/hardware/protocol/loconet/messages.cpp
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

#include "messages.hpp"
#include "../../../utils/tohex.hpp"

namespace LocoNet {

uint8_t calcChecksum(const Message& message)
{
  const uint8_t* p = reinterpret_cast<const uint8_t*>(&message);
  const int size = message.size() - 1;
  uint8_t checksum = 0xFF;
  for(int i = 0; i < size; i++)
    checksum ^= p[i];
  return checksum;
}

void updateChecksum(Message& message)
{
  reinterpret_cast<uint8_t*>(&message)[message.size() - 1] = calcChecksum(message);
}

bool isChecksumValid(const Message& message)
{
  return calcChecksum(message) == reinterpret_cast<const uint8_t*>(&message)[message.size() - 1];
}

bool isValid(const Message& message)
{
  const uint8_t size = message.size();
  if(size == 0 || !isChecksumValid(message))
    return false;
  for(uint8_t i = 1; i < size; i++) // bit 7 must be unset (except opcode)
    if(reinterpret_cast<const uint8_t*>(&message)[i] & 0x80)
      return false;
  return true;
}

std::string toString(const Message& message, bool raw)
{
  std::string s;
  if(std::string_view sv = toString(message.opCode); !sv.empty())
    s = sv;
  else
    s = toHex(message.opCode);

  switch(message.opCode)
  {
    case OPC_GPON:
    case OPC_GPOFF:
    case OPC_IDLE:
    case OPC_BUSY:
      break;

    case OPC_LOCO_SPD:
    {
      const LocoSpd& locoSpd = static_cast<const LocoSpd&>(message);
      s.append(" slot=").append(std::to_string(locoSpd.slot));
      s.append(" speed=").append(std::to_string(locoSpd.speed));
      break;
    }
    case OPC_LOCO_DIRF:
    {
      const LocoDirF& locoDirF = static_cast<const LocoDirF&>(message);
      s.append(" slot=").append(std::to_string(locoDirF.slot));
      s.append(" dir=").append(locoDirF.direction() == Direction::Forward ? "fwd" : "rev");
      s.append(" f0=").append(locoDirF.f0() ? "on" : "off");
      s.append(" f1=").append(locoDirF.f1() ? "on" : "off");
      s.append(" f2=").append(locoDirF.f2() ? "on" : "off");
      s.append(" f3=").append(locoDirF.f3() ? "on" : "off");
      s.append(" f4=").append(locoDirF.f4() ? "on" : "off");
      break;
    }
    case OPC_LOCO_SND:
    {
      const LocoSnd& locoSnd = static_cast<const LocoSnd&>(message);
      s.append(" slot=").append(std::to_string(locoSnd.slot));
      s.append(" f5=").append(locoSnd.f5() ? "on" : "off");
      s.append(" f6=").append(locoSnd.f6() ? "on" : "off");
      s.append(" f7=").append(locoSnd.f7() ? "on" : "off");
      s.append(" f8=").append(locoSnd.f8() ? "on" : "off");
      break;
    }
    case OPC_LOCO_F9F12:
    {
      const LocoF9F12& locoF9F12 = static_cast<const LocoF9F12&>(message);
      s.append(" slot=").append(std::to_string(locoF9F12.slot));
      s.append(" f9=").append(locoF9F12.f9() ? "on" : "off");
      s.append(" f10=").append(locoF9F12.f10() ? "on" : "off");
      s.append(" f11=").append(locoF9F12.f11() ? "on" : "off");
      s.append(" f12=").append(locoF9F12.f12() ? "on" : "off");
      break;
    }
    case OPC_INPUT_REP:
    {
      const InputRep& inputRep = static_cast<const InputRep&>(message);
      s.append(" fullAddress=").append(std::to_string(inputRep.fullAddress()));
      s.append(" address=").append(std::to_string(inputRep.address()));
      s.append(" input=").append(inputRep.isAuxInput() ? "aux" : "switch");
      s.append(" value=").append(inputRep.value() ? "high" : "low");
      break;
    }
    case OPC_RQ_SL_DATA:
    {
      const RequestSlotData& requestSlotData = static_cast<const RequestSlotData&>(message);
      s.append(" slot=").append(std::to_string(requestSlotData.slot));
      break;
    }
    case OPC_MULTI_SENSE:
    {
      const MultiSense& multiSense = static_cast<const MultiSense&>(message);
      if(multiSense.isTransponder())
      {
        const MultiSenseTransponder& multiSenseTransponder = static_cast<const MultiSenseTransponder&>(multiSense);
        s.append(multiSenseTransponder.isPresent() ? " present" : " absent");
        s.append(" sensorAddress=").append(std::to_string(multiSenseTransponder.sensorAddress()));
        s.append(" transponderAddress=").append(std::to_string(multiSenseTransponder.transponderAddress()));
      }
      else
        raw = true;
      break;
    }
    case OPC_D4:
    {
      const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&message);
      if(bytes[1] == 0x20)
      {
        switch(bytes[3])
        {
          case 0x08:
          {
            const LocoF13F19& locoF13F19 = static_cast<const LocoF13F19&>(message);
            s.append(" slot=").append(std::to_string(locoF13F19.slot));
            s.append(" f13=").append(locoF13F19.f13() ? "on" : "off");
            s.append(" f14=").append(locoF13F19.f14() ? "on" : "off");
            s.append(" f15=").append(locoF13F19.f15() ? "on" : "off");
            s.append(" f16=").append(locoF13F19.f16() ? "on" : "off");
            s.append(" f17=").append(locoF13F19.f17() ? "on" : "off");
            s.append(" f18=").append(locoF13F19.f18() ? "on" : "off");
            s.append(" f19=").append(locoF13F19.f19() ? "on" : "off");
            break;
          }
          case 0x05:
          {
            const LocoF20F28& locoF20F28 = static_cast<const LocoF20F28&>(message);
            s.append(" slot=").append(std::to_string(locoF20F28.slot));
            s.append(" f20=").append(locoF20F28.f20() ? "on" : "off");
            s.append(" f28=").append(locoF20F28.f28() ? "on" : "off");
            break;
          }
          case 0x09:
          {
            const LocoF21F27& locoF21F27 = static_cast<const LocoF21F27&>(message);
            s.append(" slot=").append(std::to_string(locoF21F27.slot));
            s.append(" f21=").append(locoF21F27.f21() ? "on" : "off");
            s.append(" f22=").append(locoF21F27.f22() ? "on" : "off");
            s.append(" f23=").append(locoF21F27.f23() ? "on" : "off");
            s.append(" f24=").append(locoF21F27.f24() ? "on" : "off");
            s.append(" f25=").append(locoF21F27.f25() ? "on" : "off");
            s.append(" f26=").append(locoF21F27.f26() ? "on" : "off");
            s.append(" f27=").append(locoF21F27.f27() ? "on" : "off");
            break;
          }
          default:
            raw = true;
            break;
        }
      }
      else
        raw = true;
      break;
    }
    case OPC_MULTI_SENSE_LONG:
    {
      const MultiSenseLong& multiSense = static_cast<const MultiSenseLong&>(message);
      if(multiSense.isTransponder())
      {
        const MultiSenseLongTransponder& multiSenseTransponder = static_cast<const MultiSenseLongTransponder&>(multiSense);
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
      s.append(toHex(bytes[i]));
    }
    s.append("]");
  }

  return s;
}

}
