/**
 * shared/src/enum/interfacestate.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_INTERFACESTATE_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_INTERFACESTATE_HPP

#include <cstdint>
#include <array>
#include "enum.hpp"

enum class InterfaceState : uint8_t
{
  Offline = 0,
  Initializing = 1,
  Online = 2,
  Error = 255,
};

TRAINTASTIC_ENUM(InterfaceState, "interface_state", 4,
{
  {InterfaceState::Offline, "offline"},
  {InterfaceState::Initializing, "initializing"},
  {InterfaceState::Online, "online"},
  {InterfaceState::Error, "error"},
});

inline constexpr std::array<InterfaceState, 4> interfaceStateValues{{
  InterfaceState::Offline,
  InterfaceState::Initializing,
  InterfaceState::Online,
  InterfaceState::Error,
}};

#endif
