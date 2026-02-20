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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_CBUSGETMINORPRIORITY_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_CBUSGETMINORPRIORITY_HPP

#include "cbusopcode.hpp"
#include "cbuspriority.hpp"

namespace CBUS {

constexpr MinorPriority getMinorPriority(OpCode opCode)
{
  switch(opCode)
  {
    using enum OpCode;

    case HLT:
    case ARST:
    case RESTP:
      return MinorPriority::High;

    case BON:
    case TOF:
    case TON:
    case ESTOP:
    case RTOF:
    case RTON:
      return MinorPriority::AboveNormal;

    case ACK:
    case NAK:
    case RSTAT:
    case RQMN:
    case KLOC:
    case QLOC:
    case DKEEP:
    case DBG1:
    case RLOC:
    case QCON:
    case ALOC:
    case STMOD:
    case PCON:
    case KCON:
    case DSPD:
    case DFLG:
    case DFNON:
    case DFNOF:
    case DFUN:
    case GLOC:
    case ERR:
    case RDCC3:
    case WCVO:
    case WCVB:
    case QCVS:
    case PCVS:
    case RDCC4:
    case WCVS:
    case RDCC5:
    case WCVOA:
    case CABDAT:
    case FCLK:
    case RDCC6:
    case PLOC:
    case STAT:
      return MinorPriority::Normal;

    case QNN:
    case RQNP:
    case EXTC:
    case SNN:
    case SSTAT:
    case NNRSM:
    case RQNN:
    case NNREL:
    case NNACK:
    case NNLRN:
    case NNULN:
    case NNCLR:
    case NNEVN:
    case NERD:
    case RQEVN:
    case WRACK:
    case RQDAT:
    case RQDDS:
    case BOOTM:
    case ENUM:
    case NNRST:
    case EXTC1:
    case CMDERR:
    case EVNLF:
    case NVRD:
    case NENRD:
    case RQNPN:
    case NUMEV:
    case CANID:
    case EXTC2:
    case ACON:
    case ACOF:
    case AREQ:
    case ARON:
    case AROF:
    case EVULN:
    case NVSET:
    case NVANS:
    case ASON:
    case ASOF:
    case ASRQ:
    case PARAN:
    case REVAL:
    case ARSON:
    case ARSOF:
    case EXTC3:
    case ACON1:
    case ACOF1:
    case REQEV:
    case ARON1:
    case AROF1:
    case NEVAL:
    case PNN:
    case ASON1:
    case ASOF1:
    case ARSON1:
    case ARSOF1:
    case EXTC4:
    case NAME:
    case PARAMS:
    case ACON3:
    case ACOF3:
    case ENRSP:
    case ARON3:
    case AROF3:
    case EVLRNI:
    case ACDAT:
    case ARDAT:
    case ASON3:
    case ASOF3:
    case DDES:
    case DDRS:
    case ARSON3:
    case ARSOF3:
    case EXTC6:
      return MinorPriority::Low;
  }
  return MinorPriority::Low;
}

}

#endif
