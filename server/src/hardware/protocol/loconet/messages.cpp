/**
 * server/src/hardware/protocol/loconet/messages.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2024 Reinder Feenstra
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
#include "../../../utils/inrange.hpp"
#include "../../../utils/tohex.hpp"

namespace LocoNet {

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

bool isLocoSlot(uint8_t slot)
{
  return inRange(slot, SLOT_LOCO_MIN, SLOT_LOCO_MAX);
}

void setSlot(Message& message, uint8_t slot)
{
  assert(slot < 128);

  switch(message.opCode)
  {
    case OPC_LOCO_SPD:
    case OPC_LOCO_DIRF:
    case OPC_LOCO_SND:
    case OPC_LOCO_F9F12:
      static_cast<SlotMessage&>(message).slot = slot;
      return;

    case OPC_RQ_SL_DATA:
      static_cast<RequestSlotData&>(message).slot = slot;
      return;

    case OPC_D4:
    {
      auto* bytes = reinterpret_cast<uint8_t*>(&message);
      if(bytes[1] == 0x20 && (bytes[3] == 0x08 || bytes[3] == 0x05 || bytes[3] == 0x09)) // LocoF13F19 or LocoF12F20F28 or LocoF21F27
      {
        bytes[2] = slot;
        return;
      }
      break;
    }
    case OPC_GPON:
    case OPC_GPOFF:
    case OPC_IDLE:
    case OPC_BUSY:
    case OPC_INPUT_REP:
    case OPC_SW_REQ:
    case OPC_MULTI_SENSE:
    case OPC_MULTI_SENSE_LONG:
    case OPC_SW_REP:
    case OPC_LONG_ACK:
    case OPC_SLOT_STAT1:
    case OPC_CONSIST_FUNC:
    case OPC_UNLINK_SLOTS:
    case OPC_LINK_SLOTS:
    case OPC_MOVE_SLOTS:
    case OPC_SW_STATE:
    case OPC_SW_ACK:
    case OPC_LOCO_ADR:
    case OPC_E4:
    case OPC_PEER_XFER:
    case OPC_SL_RD_DATA:
    case OPC_IMM_PACKET:
    case OPC_WR_SL_DATA:
      break; // no slot or not yet implemented
  }
  assert(false);
}

bool hasResponse(const Message& message)
{
  if(message.opCode == OPC_IMM_PACKET)
  {
    return
      isSignatureMatch<LocoF9F12IMMShortAddress>(message) ||
      isSignatureMatch<LocoF9F12IMMLongAddress>(message) ||
      isSignatureMatch<LocoF13F20IMMShortAddress>(message) ||
      isSignatureMatch<LocoF13F20IMMLongAddress>(message) ||
      isSignatureMatch<LocoF21F28IMMShortAddress>(message) ||
      isSignatureMatch<LocoF21F28IMMLongAddress>(message) ||
      Uhlenbrock::ReadSpecialOption::check(message) ||
      Uhlenbrock::LNCVStart::check(message) ||
      Uhlenbrock::LNCVRead::check(message) ||
      Uhlenbrock::LNCVWrite::check(message);
  }

  return (message.opCode & 0x08);
}

bool isValidResponse(const Message& request, const Message& response)
{
  if(response.opCode == OPC_LONG_ACK)
    return request.opCode == static_cast<const LongAck&>(response).respondingOpCode();

  switch(request.opCode)
  {
    case OPC_UNLINK_SLOTS:
      assert(false); //! @todo implement
      return false;

    case OPC_LINK_SLOTS:
      assert(false); //! @todo implement
      return false;

    case OPC_MOVE_SLOTS:
      assert(false); //! @todo implement
      return false;

    case OPC_RQ_SL_DATA:
      return
        response.opCode == OPC_SL_RD_DATA &&
        static_cast<const RequestSlotData&>(request).slot == static_cast<const SlotReadData&>(response).slot;

    case OPC_SW_STATE:
      assert(false); //! @todo implement
      return false;

    case OPC_SW_ACK:
      assert(false); //! @todo implement
      return false;

    case OPC_LOCO_ADR:
      return
        response.opCode == OPC_SL_RD_DATA &&
        static_cast<const LocoAdr&>(request).addressLow == static_cast<const SlotReadData&>(response).adr &&
        static_cast<const LocoAdr&>(request).addressHigh == static_cast<const SlotReadData&>(response).adr2;

    case OPC_IMM_PACKET:
      if(Uhlenbrock::ReadSpecialOption::check(request))
      {
        return
          Uhlenbrock::ReadSpecialOptionReply::check(response) &&
          static_cast<const Uhlenbrock::ReadSpecialOption&>(request).specialOption() == static_cast<const Uhlenbrock::ReadSpecialOptionReply&>(response).specialOption();
      }
      else if(Uhlenbrock::LNCVReadResponse::check(response))
      {
        if(Uhlenbrock::LNCVStart::check(request))
        {
          const auto& lncvRequest = static_cast<const Uhlenbrock::LNCVStart&>(request);
          const auto& lncvResponse = static_cast<const Uhlenbrock::LNCVReadResponse&>(response);

          return
            lncvRequest.moduleId() == lncvResponse.moduleId() &&
            lncvResponse.lncv() == 0;
        }
        if(Uhlenbrock::LNCVRead::check(request))
        {
          const auto& lncvRequest = static_cast<const Uhlenbrock::LNCVRead&>(request);
          const auto& lncvResponse = static_cast<const Uhlenbrock::LNCVReadResponse&>(response);

          return
            lncvRequest.moduleId() == lncvResponse.moduleId() &&
            lncvRequest.lncv() == lncvResponse.lncv();
        }
      }
      else
        assert(false); //! @todo implement
      return false;

    case OPC_WR_SL_DATA:
      assert(false); //! @todo implement
      return false;

    case OPC_BUSY:
    case OPC_GPOFF:
    case OPC_GPON:
    case OPC_IDLE:
    case OPC_LOCO_SPD:
    case OPC_LOCO_DIRF:
    case OPC_LOCO_SND:
    case OPC_LOCO_F9F12:
    case OPC_SW_REQ:
    case OPC_SW_REP:
    case OPC_INPUT_REP:
    case OPC_LONG_ACK:
    case OPC_SLOT_STAT1:
    case OPC_CONSIST_FUNC:
    case OPC_MULTI_SENSE:
    case OPC_D4:
    case OPC_MULTI_SENSE_LONG:
    case OPC_E4:
    case OPC_PEER_XFER:
    case OPC_SL_RD_DATA:
      assert(!hasResponse(request));
      break;
  }
  return false;
}

std::string toString(const Message& message)
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
      const auto& locoSpd = static_cast<const LocoSpd&>(message);
      s.append(" slot=").append(std::to_string(locoSpd.slot));
      s.append(" speed=").append(std::to_string(locoSpd.speed));
      break;
    }
    case OPC_LOCO_DIRF:
    {
      const auto& locoDirF = static_cast<const LocoDirF&>(message);
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
      const auto& locoSnd = static_cast<const LocoSnd&>(message);
      s.append(" slot=").append(std::to_string(locoSnd.slot));
      s.append(" f5=").append(locoSnd.f5() ? "on" : "off");
      s.append(" f6=").append(locoSnd.f6() ? "on" : "off");
      s.append(" f7=").append(locoSnd.f7() ? "on" : "off");
      s.append(" f8=").append(locoSnd.f8() ? "on" : "off");
      break;
    }
    case OPC_LOCO_F9F12:
    {
      const auto& locoF9F12 = static_cast<const LocoF9F12&>(message);
      s.append(" slot=").append(std::to_string(locoF9F12.slot));
      s.append(" f9=").append(locoF9F12.f9() ? "on" : "off");
      s.append(" f10=").append(locoF9F12.f10() ? "on" : "off");
      s.append(" f11=").append(locoF9F12.f11() ? "on" : "off");
      s.append(" f12=").append(locoF9F12.f12() ? "on" : "off");
      break;
    }
    case OPC_INPUT_REP:
    {
      const auto& inputRep = static_cast<const InputRep&>(message);
      s.append(" fullAddress=").append(std::to_string(inputRep.fullAddress()));
      s.append(" address=").append(std::to_string(inputRep.address()));
      s.append(" input=").append(inputRep.isAuxInput() ? "aux" : "switch");
      s.append(" value=").append(inputRep.value() ? "high" : "low");
      break;
    }
    case OPC_SW_REQ:
    {
      const auto& switchRequest = static_cast<const SwitchRequest&>(message);
      s.append(" address=").append(std::to_string(switchRequest.address()));
      s.append(" dir=").append(switchRequest.dir() ? "closed/green" : "thrown/red");
      s.append(" on=").append(switchRequest.on() ? "high" : "low");
      break;
    }
    case OPC_RQ_SL_DATA:
    {
      const auto& requestSlotData = static_cast<const RequestSlotData&>(message);
      s.append(" slot=").append(std::to_string(requestSlotData.slot));
      break;
    }
    case OPC_MULTI_SENSE:
    {
      const auto& multiSense = static_cast<const MultiSense&>(message);
      if(multiSense.isTransponder())
      {
        const auto& multiSenseTransponder = static_cast<const MultiSenseTransponder&>(multiSense);
        s.append(multiSenseTransponder.isPresent() ? " present" : " absent");
        s.append(" sensorAddress=").append(std::to_string(multiSenseTransponder.sensorAddress()));
        s.append(" transponderAddress=").append(std::to_string(multiSenseTransponder.transponderAddress()));
      }
      break;
    }
    case OPC_D4:
    {
      const auto* bytes = reinterpret_cast<const uint8_t*>(&message);
      if(bytes[1] == 0x20)
      {
        switch(bytes[3])
        {
          case 0x08:
          {
            const auto& locoF13F19 = static_cast<const LocoF13F19&>(message);
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
            const auto& locoF12F20F28 = static_cast<const LocoF12F20F28&>(message);
            s.append(" slot=").append(std::to_string(locoF12F20F28.slot));
            s.append(" f12=").append(locoF12F20F28.f12() ? "on" : "off");
            s.append(" f20=").append(locoF12F20F28.f20() ? "on" : "off");
            s.append(" f28=").append(locoF12F20F28.f28() ? "on" : "off");
            break;
          }
          case 0x09:
          {
            const auto& locoF21F27 = static_cast<const LocoF21F27&>(message);
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
        }
      }
      break;
    }
    case OPC_MULTI_SENSE_LONG:
    {
      const auto& multiSense = static_cast<const MultiSenseLong&>(message);
      s.append(::toString(multiSense.code()));
      s.append(" sensorAddress=").append(std::to_string(multiSense.sensorAddress()));
      s.append(" transponderAddress=").append(std::to_string(multiSense.transponderAddress()));
      s.append(" transponderDirection=").append(multiSense.transponderDirection() == Direction::Forward ? "fwd" : "rev");
      if(multiSense.code() == MultiSenseLong::Code::RailComAppDyn)
      {
        const auto& multiSenseRailComAppDyn = static_cast<const MultiSenseLongRailComAppDyn&>(multiSense);
        s.append(" app_dyn=").append(std::to_string(static_cast<uint8_t>(multiSenseRailComAppDyn.appDynId())));
        if(auto sv = ::toString(multiSenseRailComAppDyn.appDynId()); !sv.empty())
        {
          s.append(" ()").append(sv).append(")");
        }
        if(RailCom::isAppDynActualSpeed(multiSenseRailComAppDyn.appDynId()))
        {
          s.append(" actual_speed=").append(std::to_string(static_cast<const MultiSenseLongRailComAppDynActualSpeed&>(multiSenseRailComAppDyn).actualSpeed()));
        }
        else
        {
          s.append(" value=").append(std::to_string(multiSenseRailComAppDyn.value()));
        }
      }
      break;
    }
    case OPC_E4:
    {
      switch(static_cast<const Uhlenbrock::Lissy&>(message).type())
      {
        case Uhlenbrock::Lissy::Type::AddressCategoryDirection:
        {
          const auto& lissy = static_cast<const Uhlenbrock::LissyAddressCategoryDirection&>(message);
          s.append(" LISSY:");
          s.append(" sensorAddress=").append(std::to_string(lissy.sensorAddress()));
          s.append(" decoderAddress=").append(std::to_string(lissy.decoderAddress()));
          s.append(" category=").append(std::to_string(lissy.category()));
          switch(lissy.direction())
          {
            case Direction::Forward:
              s.append(" direction=fwd");
              break;

            case Direction::Reverse:
              s.append(" direction=rev");
              break;

            case Direction::Unknown:
              break;
          }
          break;
        }
        case Uhlenbrock::Lissy::Type::Speed:
        {
          const auto& lissy = static_cast<const Uhlenbrock::LissySpeed&>(message);
          s.append(" LISSY:");
          s.append(" sensorAddress=").append(std::to_string(lissy.sensorAddress()));
          s.append(" speed=").append(std::to_string(lissy.speed()));
          break;
        }
      }
      break;
    }
    case OPC_PEER_XFER:
    {
      if(Uhlenbrock::LNCVReadResponse::check(message))
      {
        const auto& lncvReadResponse = static_cast<const Uhlenbrock::LNCVReadResponse&>(message);
        s.append(" LNCV read response:");
        s.append(" module=").append(std::to_string(lncvReadResponse.moduleId()));
        s.append(" lncv=").append(std::to_string(lncvReadResponse.lncv()));
        s.append(" value=").append(std::to_string(lncvReadResponse.value()));
      }
      break;
    }
    case OPC_SL_RD_DATA:
    case OPC_WR_SL_DATA:
    {
      const auto& slotData = static_cast<const SlotDataBase&>(message);
      if(slotData.slot == SLOT_FAST_CLOCK)
      {
        const auto& fastClock = static_cast<const FastClockSlotData&>(message);
        s.append(" slot=fastclock");
        s.append(" clk_rate=").append(std::to_string(fastClock.clk_rate));
        s.append(" days=").append(std::to_string(fastClock.days));
        s.append(" hour=").append(std::to_string(fastClock.hour()));
        s.append(" minute=").append(std::to_string(fastClock.minute()));
        s.append(fastClock.valid() ? " valid" : " invalid");
        s.append(" id=").append(std::to_string(fastClock.id()));
        s.append(" frac_minsl=").append(std::to_string(fastClock.frac_minsl));
        s.append(" frac_minsh=").append(std::to_string(fastClock.frac_minsh));
        s.append(" trk=").append(std::to_string(fastClock.trk));
      }
      break;
    }
    case OPC_IMM_PACKET:
    {
      if(isSignatureMatch<LocoF9F12IMMShortAddress>(message))
      {
        const auto& locoF9F12 = static_cast<const LocoF9F12IMMShortAddress&>(message);
        s.append(" short_address=").append(std::to_string(locoF9F12.address()));
        s.append(" f9=").append(locoF9F12.f9() ? "on" : "off");
        s.append(" f10=").append(locoF9F12.f10() ? "on" : "off");
        s.append(" f11=").append(locoF9F12.f11() ? "on" : "off");
        s.append(" f12=").append(locoF9F12.f12() ? "on" : "off");
      }
      else if(isSignatureMatch<LocoF9F12IMMLongAddress>(message))
      {
        const auto& locoF9F12 = static_cast<const LocoF9F12IMMLongAddress&>(message);
        s.append(" long_address=").append(std::to_string(locoF9F12.address()));
        s.append(" f9=").append(locoF9F12.f9() ? "on" : "off");
        s.append(" f10=").append(locoF9F12.f10() ? "on" : "off");
        s.append(" f11=").append(locoF9F12.f11() ? "on" : "off");
        s.append(" f12=").append(locoF9F12.f12() ? "on" : "off");
      }
      else if(isSignatureMatch<LocoF13F20IMMShortAddress>(message))
      {
        const auto& locoF13F20 = static_cast<const LocoF13F20IMMShortAddress&>(message);
        s.append(" short_address=").append(std::to_string(locoF13F20.address()));
        s.append(" f13=").append(locoF13F20.f13() ? "on" : "off");
        s.append(" f14=").append(locoF13F20.f14() ? "on" : "off");
        s.append(" f15=").append(locoF13F20.f15() ? "on" : "off");
        s.append(" f16=").append(locoF13F20.f16() ? "on" : "off");
        s.append(" f17=").append(locoF13F20.f17() ? "on" : "off");
        s.append(" f18=").append(locoF13F20.f18() ? "on" : "off");
        s.append(" f19=").append(locoF13F20.f19() ? "on" : "off");
        s.append(" f20=").append(locoF13F20.f20() ? "on" : "off");
      }
      else if(isSignatureMatch<LocoF13F20IMMLongAddress>(message))
      {
        const auto& locoF13F20 = static_cast<const LocoF13F20IMMLongAddress&>(message);
        s.append(" long_address=").append(std::to_string(locoF13F20.address()));
        s.append(" f13=").append(locoF13F20.f13() ? "on" : "off");
        s.append(" f14=").append(locoF13F20.f14() ? "on" : "off");
        s.append(" f15=").append(locoF13F20.f15() ? "on" : "off");
        s.append(" f16=").append(locoF13F20.f16() ? "on" : "off");
        s.append(" f17=").append(locoF13F20.f17() ? "on" : "off");
        s.append(" f18=").append(locoF13F20.f18() ? "on" : "off");
        s.append(" f19=").append(locoF13F20.f19() ? "on" : "off");
        s.append(" f20=").append(locoF13F20.f20() ? "on" : "off");
      }
      else if(isSignatureMatch<LocoF21F28IMMShortAddress>(message))
      {
        const auto& locoF21F28 = static_cast<const LocoF21F28IMMShortAddress&>(message);
        s.append(" short_address=").append(std::to_string(locoF21F28.address()));
        s.append(" f21=").append(locoF21F28.f21() ? "on" : "off");
        s.append(" f22=").append(locoF21F28.f22() ? "on" : "off");
        s.append(" f23=").append(locoF21F28.f23() ? "on" : "off");
        s.append(" f24=").append(locoF21F28.f24() ? "on" : "off");
        s.append(" f25=").append(locoF21F28.f25() ? "on" : "off");
        s.append(" f26=").append(locoF21F28.f26() ? "on" : "off");
        s.append(" f27=").append(locoF21F28.f27() ? "on" : "off");
        s.append(" f28=").append(locoF21F28.f28() ? "on" : "off");
      }
      else if(isSignatureMatch<LocoF21F28IMMLongAddress>(message))
      {
        const auto& locoF21F28 = static_cast<const LocoF21F28IMMLongAddress&>(message);
        s.append(" long_address=").append(std::to_string(locoF21F28.address()));
        s.append(" f21=").append(locoF21F28.f21() ? "on" : "off");
        s.append(" f22=").append(locoF21F28.f22() ? "on" : "off");
        s.append(" f23=").append(locoF21F28.f23() ? "on" : "off");
        s.append(" f24=").append(locoF21F28.f24() ? "on" : "off");
        s.append(" f25=").append(locoF21F28.f25() ? "on" : "off");
        s.append(" f26=").append(locoF21F28.f26() ? "on" : "off");
        s.append(" f27=").append(locoF21F28.f27() ? "on" : "off");
        s.append(" f28=").append(locoF21F28.f28() ? "on" : "off");
      }
      else if(Uhlenbrock::LNCVStart::check(message))
      {
        const auto& lncvStart = static_cast<const Uhlenbrock::LNCVStart&>(message);
        s.append(" LNCV start:");
        s.append(" module=").append(std::to_string(lncvStart.moduleId()));
        s.append(" address=").append(std::to_string(lncvStart.address()));
      }
      else if(Uhlenbrock::LNCVRead::check(message))
      {
        const auto& lncvRead = static_cast<const Uhlenbrock::LNCVRead&>(message);
        s.append(" LNCV read:");
        s.append(" module=").append(std::to_string(lncvRead.moduleId()));
        s.append(" address=").append(std::to_string(lncvRead.address()));
        s.append(" lncv=").append(std::to_string(lncvRead.lncv()));
      }
      else if(Uhlenbrock::LNCVWrite::check(message))
      {
        const auto& lncvWrite = static_cast<const Uhlenbrock::LNCVWrite&>(message);
        s.append(" LNCV write:");
        s.append(" module=").append(std::to_string(lncvWrite.moduleId()));
        s.append(" lncv=").append(std::to_string(lncvWrite.lncv()));
        s.append(" value=").append(std::to_string(lncvWrite.value()));
      }
      else if(Uhlenbrock::LNCVStop::check(message))
      {
        const auto& lncvStop = static_cast<const Uhlenbrock::LNCVStop&>(message);
        s.append(" LNCV stop:");
        s.append(" module=").append(std::to_string(lncvStop.moduleId()));
        s.append(" address=").append(std::to_string(lncvStop.address()));
      }
      break;
    }
    default:
      break;
  }

  // raw bytes:
  s.append(" [");
  const auto* bytes = reinterpret_cast<const uint8_t*>(&message);
  for(int i = 0; i < message.size(); i++)
  {
    if(i != 0)
      s.append(" ");
    s.append(toHex(bytes[i]));
  }
  s.append("]");

  return s;
}

}
