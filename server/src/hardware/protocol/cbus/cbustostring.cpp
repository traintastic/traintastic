/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#include "cbustostring.hpp"
#include "cbusmessages.hpp"
#include "../../../compat/stdformat.hpp"
#include "../../../utils/tohex.hpp"

namespace CBUS {

namespace {

template<typename T>
requires(std::is_base_of_v<Message, T>)
std::string formatEngineAddress(const T& message)
{
  return std::format(" {}={}", message.isLongAddress() ? "long_address" : "short_address", message.address());
}

constexpr std::string_view toString(GetEngineSession::Mode mode) noexcept
{
  switch(mode)
  {
    using enum GetEngineSession::Mode;

    case Request:
      return "request";

    case Steal:
      return "steal";

    case Share:
      return "share";
  }
  return {};
}

constexpr std::string_view toString(SetEngineSessionMode::SpeedMode mode) noexcept
{
  switch(mode)
  {
    case SetEngineSessionMode::SpeedMode128:
      return "128";

    case SetEngineSessionMode::SpeedMode14:
      return "14";

    case SetEngineSessionMode::SpeedMode28WithInterleaveSteps:
      return "28+interleave";

    case SetEngineSessionMode::SpeedMode28:
      return "28";
  }
  return {};
}

}

std::string toString(const Message& message)
{
  std::string s(toString(message.opCode));

  switch (message.opCode)
  {
    using enum OpCode;

    // 00-1F – 0 data byte packets:
    case ACK:
    case NAK:
    case HLT:
    case BON:
    case TOF:
    case TON:
    case ESTOP:
    case ARST:
    case RTOF:
    case RTON:
    case RESTP:
    case RSTAT:
    case QNN:
    case RQNP:
    case RQMN:
      break; // no data

    // 20–3F - 1 data byte packets:
    case KLOC:
    case QLOC:
    case DKEEP:
    {
      const auto& m = static_cast<const EngineSessionMessage&>(message);
      s.append(std::format(" session={}", m.session));
      break;
    }
    case DBG1:
      break;

    case EXTC:
      break;

    // 40–5F - 2 data byte packets:
    case RLOC:
    {
      const auto& m = static_cast<const RequestEngineSession&>(message);
      s.append(formatEngineAddress(m));
      break;
    }
    case QCON:
      break;

    case SNN:
      break;

    case ALOC:
      break;

    case STMOD:
    {
      const auto& m = static_cast<const SetEngineSessionMode&>(message);
      s.append(std::format(" session={} speed_mode={} service_mode={} sound_control_mode={}", m.session, toString(m.speedMode()), m.serviceMode(), m.soundControlMode()));
      break;
    }
    case PCON:
      break;

    case KCON:
      break;

    case DSPD:
    {
      const auto& m = static_cast<const SetEngineSpeedDirection&>(message);
      s.append(std::format(" session={} speed={} direction={}", m.session, m.speed(), m.directionForward() ? "fwd" : "rev"));
      break;
    }
    case DFLG:
      break;

    case DFNON:
    case DFNOF:
    {
      const auto& m = static_cast<const SetEngineFunction&>(message);
      s.append(std::format(" session={} number={}", m.session, m.number));
      break;
    }
    case SSTAT:
      break;

    case NNRSM:
      break;

    case RQNN:
      break;

    case NNREL:
      break;

    case NNACK:
      break;

    case NNLRN:
      break;

    case NNULN:
      break;

    case NNCLR:
      break;

    case NNEVN:
      break;

    case NERD:
      break;

    case RQEVN:
      break;

    case WRACK:
      break;

    case RQDAT:
      break;

    case RQDDS:
      break;

    case BOOTM:
      break;

    case ENUM:
      break;

    case NNRST:
      break;

    case EXTC1:
      break;

    // 60-7F - 3 data byte packets:
    case DFUN:
      break;

    case GLOC:
    {
      const auto& m = static_cast<const GetEngineSession&>(message);
      s.append(formatEngineAddress(m));
      s.append(std::format(" mode={}", toString(m.mode())));
      break;
    }
    case ERR:
    {
      const auto& m = static_cast<const CommandStationErrorMessage&>(message);
      s.append(std::format(" error={}", toString(m.errorCode)));
      switch(m.errorCode)
      {
        using enum DCCErr;

        case LocoStackFull:
        case LocoAddressTaken:
        case InvalidRequest:
        {
          s.append(formatEngineAddress(static_cast<const CommandStationLocoStackFullError&>(message)));
          break;
        }
        case SessionNotPresent:
        case LocoNotFound:
        case SessionCancelled:
        {
          s.append(std::format(" session={}", static_cast<const CommandStationSessionNotPresentError&>(message).session()));
          break;
        }
        case ConsistEmpty:
        {
          s.append(std::format(" consist={}", static_cast<const CommandStationConsistEmptyError&>(message).consist()));
          break;
        }
        case CANBusError:
          break;
      }
      break;
    }
    case CMDERR:
      break;

    case EVNLF:
      break;

    case NVRD:
      break;

    case NENRD:
      break;

    case RQNPN:
    {
      const auto& m = static_cast<const ReadNodeParameter&>(message);
      s.append(std::format(" node={} param={}", m.nodeNumber(), static_cast<uint8_t>(m.parameter)));
      break;
    }
    case NUMEV:
      break;

    case CANID:
      break;

    case EXTC2:
      break;

    // 80-9F - 4 data byte packets:
    case RDCC3:
      break;

    case WCVO:
      break;

    case WCVB:
      break;

    case QCVS:
      break;

    case PCVS:
      break;

    case ACON:
    case ACOF:
    case AREQ:
    case ARON:
    case AROF:
    {
      const auto& m = static_cast<const AccessoryRequestEvent&>(message); // same memory layout, only opcode is different
      s.append(std::format(" node={} event={}", m.nodeNumber(), m.eventNumber()));
      break;
    }
    case EVULN:
      break;

    case NVSET:
      break;

    case NVANS:
      break;

    case ASON:
    case ASOF:
    case ASRQ:
    case ARSON:
    case ARSOF:
    {
      const auto& m = static_cast<const AccessoryShortRequestEvent&>(message); // same memory layout, only opcode is different
      s.append(std::format(" node={} device={}", m.nodeNumber(), m.deviceNumber()));
      break;
    }
    case PARAN:
    {
      const auto& m = static_cast<const NodeParameterResponse&>(message);
      s.append(std::format(" node={} param={} value={}", m.nodeNumber(), static_cast<uint8_t>(m.parameter), m.value));
      break;
    }
    case REVAL:
      break;

    case EXTC3:
      break;

    // A0-BF - 5 data byte packets:
    case RDCC4:
      break;

    case WCVS:
      break;

    case ACON1:
      break;

    case ACOF1:
      break;

    case REQEV:
      break;

    case ARON1:
      break;

    case AROF1:
      break;

    case NEVAL:
      break;

    case PNN:
    {
      const auto& m = static_cast<const PresenceOfNode&>(message);
      s.append(std::format(" node={} manufacturer_id={} module_id={} flags=0x{:02X}", m.nodeNumber(), m.manufacturerId, m.moduleId, m.flags));
      break;
    }
    case ASON1:
      break;

    case ASOF1:
      break;

    case ARSON1:
      break;

    case ARSOF1:
      break;

    case EXTC4:
      break;

    // C0-DF - 6 data byte packets:
    case RDCC5:
      break;

    case WCVOA:
      break;

    case CABDAT:
      break;

    case FCLK:
      break;

    case ACON2:
    case ACOF2:
    case ARON2:
    case AROF2:
    {
      const auto& m = static_cast<const Accessory2On&>(message); // same memory layout, only opcode is different
      s.append(std::format(" node={} event={} data={} {} ({})", m.nodeNumber(), m.eventNumber(), m.data1, m.data2, m.data()));
      break;
    }
    case EVLRN:
      break;

    case EVANS:
      break;

    case ASON2:
      break;

    case ASOF2:
      break;

    case ARSON2:
      break;

    case ARSOF2:
      break;

    case EXTC5:
      break;

    // E0-FF - 7 data byte packets:
    case RDCC6:
      break;

    case PLOC:
    {
      const auto& m = static_cast<const EngineReport&>(message);
      s.append(std::format(" session={}", m.session));
      s.append(formatEngineAddress(m));
      s.append(std::format(" speed={} direction={} f0={} f1={} f2={} f3={} f4={} f5={} f6={} f7={} f8={} f9={} f10={} f11={} f12={}",
        m.speed(),
        m.directionForward() ? "fwd" : "rev",
        m.f0(), m.f1(), m.f2(), m.f3(), m.f4(),
        m.f5(), m.f6(), m.f7(), m.f8(),
        m.f9(), m.f10(), m.f11(), m.f12()));
      break;
    }
    case NAME:
      break;

    case STAT:
    {
      const auto& m = static_cast<const CommandStationStatusReport&>(message);
      s.append(std::format(" node={} cs_num={} flags=0x{:02X} major={} minor={} build={}", m.nodeNumber(), m.csNum, m.flags, m.majorRev, m.minorRev, m.build));
      break;
    }
    case PARAMS:
      break;

    case ACON3:
      break;

    case ACOF3:
      break;

    case ENRSP:
      break;

    case ARON3:
      break;

    case AROF3:
      break;

    case EVLRNI:
      break;

    case ACDAT:
      break;

    case ARDAT:
      break;

    case ASON3:
      break;

    case ASOF3:
      break;

    case DDES:
      break;

    case DDRS:
      break;

    case ARSON3:
      break;

    case ARSOF3:
      break;

    case EXTC6:
      break;
  }

  s.append(" [");
  s.append(toHex(&message, message.size(), true));
  s.append("]");

  return s;
}

}
