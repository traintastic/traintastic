/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2024-2026 Reinder Feenstra
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
  LongEvent = 9,
  ShortEvent = 10,
};

TRAINTASTIC_ENUM(OutputChannel, "output_channel", 9,
{
  {OutputChannel::Output, "output"},
  {OutputChannel::Accessory, "accessory"},
  {OutputChannel::AccessoryDCC, "accessory_dcc"},
  {OutputChannel::AccessoryMotorola, "accessory_motorola"},
  {OutputChannel::DCCext, "dcc_ext"},
  {OutputChannel::Turnout, "turnout"},
  {OutputChannel::ECoSObject, "ecos_object"},
  {OutputChannel::LongEvent, "long_event"},
  {OutputChannel::ShortEvent, "short_event"},
});

inline constexpr std::array<OutputChannel, 9> outputChannelValues{{
  OutputChannel::Output,
  OutputChannel::Accessory,
  OutputChannel::AccessoryDCC,
  OutputChannel::AccessoryMotorola,
  OutputChannel::DCCext,
  OutputChannel::Turnout,
  OutputChannel::ECoSObject,
  OutputChannel::LongEvent,
  OutputChannel::ShortEvent,
}};

constexpr bool isAccessory(OutputChannel value)
{
  return
    (value == OutputChannel::Accessory) ||
    (value == OutputChannel::AccessoryDCC) ||
    (value == OutputChannel::AccessoryMotorola);
}

constexpr bool hasNode(OutputChannel value)
{
  return (value == OutputChannel::LongEvent);
}

template<>
struct std::hash<OutputChannel>
{
  size_t operator()(OutputChannel const& value) const noexcept
  {
    using UT = std::underlying_type_t<OutputChannel>;
    return std::hash<UT>{}(static_cast<UT>(value));
  }
};

#endif
