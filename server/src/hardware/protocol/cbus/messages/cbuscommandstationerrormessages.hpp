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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_MESSAGES_CBUSCOMMANDSTATIONERRORMESSAGES_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_MESSAGES_CBUSCOMMANDSTATIONERRORMESSAGES_HPP

#include "cbusmessage.hpp"
#include "../cbusdccerr.hpp"

namespace CBUS {

struct CommandStationErrorMessage : Message
{
  uint8_t data1;
  uint8_t data2;
  DCCErr errorCode;

protected:
  CommandStationErrorMessage(DCCErr ec, uint8_t d1, uint8_t d2)
    : Message(OpCode::ERR)
    , data1{d1}
    , data2{d2}
    , errorCode{ec}
  {
  }
};

template<DCCErr ec>
struct CommandStationGeneralErrorMessage : CommandStationErrorMessage
{
  CommandStationGeneralErrorMessage()
    : CommandStationErrorMessage(ec, 0, 0)
  {
  }
};

template<DCCErr ec>
struct CommandStationLocoAddressErrorMessage : CommandStationErrorMessage
{
  CommandStationLocoAddressErrorMessage(uint16_t address_, bool longAddress)
    : CommandStationErrorMessage(ec,
        longAddress ? (0xC0 | (address_ >> 8)) : 0,
        longAddress ? (address_ & 0xFF) : (address_ & 0x7F))
  {
  }

  bool isLongAddress() const
  {
    return (data1 & 0xC0) == 0xC0;
  }

  uint16_t address() const
  {
    return to16(data2, data1 & 0x3F);
  }
};

template<DCCErr ec>
struct CommandStationSessionErrorMessage : CommandStationErrorMessage
{
  CommandStationSessionErrorMessage(uint8_t session_)
    : CommandStationErrorMessage(ec, session_, 0)
  {
  }

  uint8_t session() const
  {
    return data1;
  }
};

template<DCCErr ec>
struct CommandStationConsistErrorMessage : CommandStationErrorMessage
{
  CommandStationConsistErrorMessage(uint8_t consist_)
    : CommandStationErrorMessage(ec, consist_, 0)
  {
  }

  uint8_t consist() const
  {
    return data1;
  }
};

using CommandStationLocoStackFullError = CommandStationLocoAddressErrorMessage<DCCErr::LocoStackFull>;
using CommandStationLocoAddressTakenError = CommandStationLocoAddressErrorMessage<DCCErr::LocoAddressTaken>;
using CommandStationSessionNotPresentError = CommandStationSessionErrorMessage<DCCErr::SessionNotPresent>;
using CommandStationConsistEmptyError = CommandStationConsistErrorMessage<DCCErr::ConsistEmpty>;
using CommandStationLocoNotFoundError = CommandStationSessionErrorMessage<DCCErr::LocoNotFound>;
using CommandStationCANBusError = CommandStationGeneralErrorMessage<DCCErr::CANBusError>;
using CommandStationInvalidRequestError = CommandStationLocoAddressErrorMessage<DCCErr::InvalidRequest>;
using CommandStationSessionCancelled = CommandStationSessionErrorMessage<DCCErr::SessionCancelled>;

}

#endif
