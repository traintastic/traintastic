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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_MESSAGES_CBUSNODEPARAMETERMESSAGES_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_MESSAGES_CBUSNODEPARAMETERMESSAGES_HPP

#include "cbusmessage.hpp"

namespace CBUS {

//! \see VLCB Opcode specification, Appendix B - Module Parameters
enum class NodeParameter : uint8_t
{
  NumberOfParameters = 0,
  ManufacturerId = 1,
  VersionMinor = 2,
  ModuleId = 3,
  NumberOfEvents = 4,
  NumberOfEventVariablesPerEvent = 5,
  NumberOfNodeVariables = 6,
  VersionMajor = 7,
  ModuleSupport = 8,
  ProcessorId = 9,
  InterfaceProtocol = 10,
  LoadAddress0 = 11,
  LoadAddress1 = 12,
  LoadAddress2 = 13,
  LoadAddress3 = 14,
  ManufacturersProcessorCode0 = 15,
  ManufacturersProcessorCode1 = 16,
  ManufacturersProcessorCode2 = 17,
  ManufacturersProcessorCode3 = 18,
  ManufacturerCode = 19,
  BetaReleaseCode = 20,
};

struct NodeParameterMessage : NodeMessage
{
  NodeParameter parameter;

protected:
  NodeParameterMessage(OpCode opc, uint16_t nodeNumber_, NodeParameter parameter_)
    : NodeMessage(opc, nodeNumber_)
    , parameter{parameter_}
  {
  }
};

struct ReadNodeParameter  : NodeParameterMessage
{
  ReadNodeParameter(uint16_t nodeNumber_, NodeParameter parameter_)
    : NodeParameterMessage(OpCode::RQNPN, nodeNumber_, parameter_)
  {
  }
};

struct NodeParameterResponse : NodeParameterMessage
{
  uint8_t value;

  NodeParameterResponse(uint16_t nodeNumber_, NodeParameter parameter_, uint8_t value_)
    : NodeParameterMessage(OpCode::PARAN, nodeNumber_, parameter_)
    , value{value_}
  {
  }
};

}

#endif
