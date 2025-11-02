/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_INPUTCHANNEL_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_INPUTCHANNEL_HPP

#include <cstdint>
#include <array>
#include "enum.hpp"

enum class InputChannel : uint16_t
{
  Input = 1, //!< Generic input (if an interface has ONE channel, use this one!)
  LocoNet = 2,
  RBus = 3,
  S88 = 4,
  S88_Left = 5,
  S88_Middle = 6,
  S88_Right = 7,
  ECoSDetector = 8,
  SX0 = 9,
  SX1 = 10,
  SX2 = 11,
};

TRAINTASTIC_ENUM(InputChannel, "input_channel", 11,
{
  {InputChannel::Input, "input"},
  {InputChannel::LocoNet, "loconet"},
  {InputChannel::RBus, "rbus"},
  {InputChannel::S88, "s88"},
  {InputChannel::S88_Left, "s88_left"},
  {InputChannel::S88_Middle, "s88_middle"},
  {InputChannel::S88_Right, "s88_right"},
  {InputChannel::ECoSDetector, "ecos_detector"},
  {InputChannel::SX0, "sx0"},
  {InputChannel::SX1, "sx1"},
  {InputChannel::SX2, "sx2"},
});

inline constexpr std::array<InputChannel, 11> inputChannelValues{{
  InputChannel::Input,
  InputChannel::LocoNet,
  InputChannel::RBus,
  InputChannel::S88,
  InputChannel::S88_Left,
  InputChannel::S88_Middle,
  InputChannel::S88_Right,
  InputChannel::ECoSDetector,
  InputChannel::SX0,
  InputChannel::SX1,
  InputChannel::SX2,
}};

#endif
