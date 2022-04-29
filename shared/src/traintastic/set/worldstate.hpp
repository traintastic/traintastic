/**
 * shared/src/set/set.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_SET_WORLDSTATE_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_SET_WORLDSTATE_HPP

#include <cstdint>
#include "set.hpp"

enum class WorldState : uint32_t
{
  Edit = 1 << 0,
  Online = 1 << 1,
  PowerOn = 1 << 2,
  Run = 1 << 3,
  Mute = 1 << 4,
  NoSmoke = 1 << 5,
  Simulation = 1 << 6,
};

TRAINTASTIC_SET(WorldState, "world_state", 7,
  (
    WorldState::Edit |
    WorldState::Online |
    WorldState::PowerOn |
    WorldState::Run |
    WorldState::Mute |
    WorldState::NoSmoke |
    WorldState::Simulation
  ),
  {
    {WorldState::Edit, "edit"},
    {WorldState::Online, "online"},
    {WorldState::PowerOn, "power_on"},
    {WorldState::Run, "run"},
    {WorldState::Mute, "mute"},
    {WorldState::NoSmoke, "no_smoke"},
    {WorldState::Simulation, "simulation"}
  });

#endif
