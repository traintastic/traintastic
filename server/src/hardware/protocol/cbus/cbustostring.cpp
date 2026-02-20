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
#include <format>
#include "cbusmessages.hpp"
#include "../../../utils/tohex.hpp"

namespace CBUS {

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
      const auto& m = static_cast<const EngineMessage&>(message);
      s.append(std::format(" session={}", m.session));
      break;
    }
    case DBG1:
      break;

    case EXTC:
      break;

    // 40–5F - 2 data byte packets:
    case RLOC:
      break;

    case QCON:
      break;

    case SNN:
      break;

    case ALOC:
      break;

    case STMOD:
      break;

    case PCON:
      break;

    case KCON:
      break;

    case DSPD:
      break;

    case DFLG:
      break;

    case DFNON:
      break;

    case DFNOF:
      break;

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
      break;

    case ERR:
      break;

    case CMDERR:
      break;

    case EVNLF:
      break;

    case NVRD:
      break;

    case NENRD:
      break;

    case RQNPN:
      break;

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
      break;

    case ACOF:
      break;

    case AREQ:
      break;

    case ARON:
      break;

    case AROF:
      break;

    case EVULN:
      break;

    case NVSET:
      break;

    case NVANS:
      break;

    case ASON:
      break;

    case ASOF:
      break;

    case ASRQ:
      break;

    case PARAN:
      break;

    case REVAL:
      break;

    case ARSON:
      break;

    case ARSOF:
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
      break;

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

    // E0-FF - 7 data byte packets:
    case RDCC6:
      break;

    case PLOC:
      break;

    case NAME:
      break;

    case STAT:
      break;

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
  s.append(toHex(&message, message.size()));
  s.append("]");

  return s;
}

}
