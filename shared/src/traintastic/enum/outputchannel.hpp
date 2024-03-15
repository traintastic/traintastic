/**
 * shared/src/traintastic/enum/outputchannel.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_OUTPUTCHANNEL_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_OUTPUTCHANNEL_HPP

#include <cstdint>
#include <array>
#include "enum.hpp"

enum class OutputChannel : uint16_t
{
  Output = 1, //!< Generic output
  Accessory = 2, //!< Accessory (use when only one protocol is supported, else use protocol specific values)
  AccessoryDCC = 3,
  AccessoryMotorola = 4,
  DCCext = 5, //!< DCCext, see RCN-213
  Turnout = 6, //!< DCC-EX turnout
  ECoSObject = 7, //!< ECoS switch object
};

TRAINTASTIC_ENUM(OutputChannel, "output_channel", 7,
{
  {OutputChannel::Output, "output"},
  {OutputChannel::Accessory, "accessory"},
  {OutputChannel::AccessoryDCC, "accessory_dcc"},
  {OutputChannel::AccessoryMotorola, "accessory_motorola"},
  {OutputChannel::DCCext, "dcc_ext"},
  {OutputChannel::Turnout, "turnout"},
  {OutputChannel::ECoSObject, "ecos_object"},
});

inline constexpr std::array<OutputChannel, 7> outputChannelValues{{
  OutputChannel::Output,
  OutputChannel::Accessory,
  OutputChannel::AccessoryDCC,
  OutputChannel::AccessoryMotorola,
  OutputChannel::DCCext,
  OutputChannel::Turnout,
  OutputChannel::ECoSObject,
}};

constexpr bool isAccessory(OutputChannel value)
{
  return
    (value == OutputChannel::Accessory) ||
    (value == OutputChannel::AccessoryDCC) ||
    (value == OutputChannel::AccessoryMotorola);
}

#endif
