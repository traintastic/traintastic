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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_CBUSTOSTRING_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_CBUSTOSTRING_HPP

#include <string_view>
#include "cbusopcode.hpp"

namespace CBUS {

struct Message;

constexpr std::string_view toString(OpCode opCode)
{
  switch(opCode)
  {
    using enum OpCode;

    // 00-1F – 0 data byte packets:
    case ACK: return "ACK";
    case NAK: return "NAK";
    case HLT: return "HLT";
    case BON: return "BON";
    case TOF: return "TOF";
    case TON: return "TON";
    case ESTOP: return "ESTOP";
    case ARST: return "ARST";
    case RTOF: return "RTOF";
    case RTON: return "RTON";
    case RESTP: return "RESTP";
    case RSTAT: return "RSTAT";
    case QNN: return "QNN";
    case RQNP: return "RQNP";
    case RQMN: return "RQMN";

    // 20–3F - 1 data byte packets:
    case KLOC: return "KLOC";
    case QLOC: return "QLOC";
    case DKEEP: return "DKEEP";
    case DBG1: return "DBG1";
    case EXTC: return "EXTC";

    // 40–5F - 2 data byte packets:
    case RLOC: return "RLOC";
    case QCON: return "QCON";
    case SNN: return "SNN";
    case ALOC: return "ALOC";
    case STMOD: return "STMOD";
    case PCON: return "PCON";
    case KCON: return "KCON";
    case DSPD: return "DSPD";
    case DFLG: return "DFLG";
    case DFNON: return "DFNON";
    case DFNOF: return "DFNOF";
    case SSTAT: return "SSTAT";
    case NNRSM: return "NNRSM";
    case RQNN: return "RQNN";
    case NNREL: return "NNREL";
    case NNACK: return "NNACK";
    case NNLRN: return "NNLRN";
    case NNULN: return "NNULN";
    case NNCLR: return "NNCLR";
    case NNEVN: return "NNEVN";
    case NERD: return "NERD";
    case RQEVN: return "RQEVN";
    case WRACK: return "WRACK";
    case RQDAT: return "RQDAT";
    case RQDDS: return "RQDDS";
    case BOOTM: return "BOOTM";
    case ENUM: return "ENUM";
    case NNRST: return "NNRST";
    case EXTC1: return "EXTC1";

    // 60-7F - 3 data byte packets:
    case DFUN: return "DFUN";
    case GLOC: return "GLOC";
    case ERR: return "ERR";
    case CMDERR: return "CMDERR";
    case EVNLF: return "EVNLF";
    case NVRD: return "NVRD";
    case NENRD: return "NENRD";
    case RQNPN: return "RQNPN";
    case NUMEV: return "NUMEV";
    case CANID: return "CANID";
    case EXTC2: return "EXTC2";

    // 80-9F - 4 data byte packets:
    case RDCC3: return "RDCC3";
    case WCVO: return "WCVO";
    case WCVB: return "WCVB";
    case QCVS: return "QCVS";
    case PCVS: return "PCVS";
    case ACON: return "ACON";
    case ACOF: return "ACOF";
    case AREQ: return "AREQ";
    case ARON: return "ARON";
    case AROF: return "AROF";
    case EVULN: return "EVULN";
    case NVSET: return "NVSET";
    case NVANS: return "NVANS";
    case ASON: return "ASON";
    case ASOF: return "ASOF";
    case ASRQ: return "ASRQ";
    case PARAN: return "PARAN";
    case REVAL: return "REVAL";
    case ARSON: return "ARSON";
    case ARSOF: return "ARSOF";
    case EXTC3: return "EXTC3";

    // A0-BF - 5 data byte packets:
    case RDCC4: return "RDCC4";
    case WCVS: return "WCVS";
    case ACON1: return "ACON1";
    case ACOF1: return "ACOF1";
    case REQEV: return "REQEV";
    case ARON1: return "ARON1";
    case AROF1: return "AROF1";
    case NEVAL: return "NEVAL";
    case PNN: return "PNN";
    case ASON1: return "ASON1";
    case ASOF1: return "ASOF1";
    case ARSON1: return "ARSON1";
    case ARSOF1: return "ARSOF1";
    case EXTC4: return "EXTC4";

    // C0-DF - 6 data byte packets:
    case RDCC5: return "RDCC5";
    case WCVOA: return "WCVOA";
    case CABDAT: return "CABDAT";
    case FCLK: return "FCLK";

    // E0-FF - 7 data byte packets:
    case RDCC6: return "RDCC6";
    case PLOC: return "PLOC";
    case NAME: return "NAME";
    case STAT: return "STAT";
    case PARAMS: return "PARAMS";
    case ACON3: return "ACON3";
    case ACOF3: return "ACOF3";
    case ENRSP: return "ENRSP";
    case ARON3: return "ARON3";
    case AROF3: return "AROF3";
    case EVLRNI: return "EVLRNI";
    case ACDAT: return "ACDAT";
    case ARDAT: return "ARDAT";
    case ASON3: return "ASON3";
    case ASOF3: return "ASOF3";
    case DDES: return "DDES";
    case DDRS: return "DDRS";
    case ARSON3: return "ARSON3";
    case ARSOF3: return "ARSOF3";
    case EXTC6: return "EXTC6";
  }
  return {};
}

std::string toString(const Message& message);

}

#endif
