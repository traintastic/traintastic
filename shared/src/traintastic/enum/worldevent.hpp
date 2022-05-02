/**
 * shared/src/enum/worldevent.hpp
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_WORLDEVENT_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_WORLDEVENT_HPP

#include <cstdint>
#include "enum.hpp"

enum class WorldEvent : uint64_t
{
  EditDisabled = 0,
  EditEnabled = 1,
  Offline = 2,
  Online = 3,
  PowerOff = 4,
  PowerOn = 5,
  Stop = 6,
  Run = 7,
  Unmute = 8,
  Mute = 9,
  NoSmoke = 10,
  Smoke = 11,
  SimulationDisabled = 12,
  SimulationEnabled = 13,
};

TRAINTASTIC_ENUM(WorldEvent, "world_event", 14,
{
  {WorldEvent::EditDisabled, "edit_disabled"},
  {WorldEvent::EditEnabled, "edit_enabled"},
  {WorldEvent::Offline, "offline"},
  {WorldEvent::Online, "online"},
  {WorldEvent::PowerOff, "power_off"},
  {WorldEvent::PowerOn, "power_on"},
  {WorldEvent::Stop, "stop"},
  {WorldEvent::Run, "run"},
  {WorldEvent::Unmute, "unmute"},
  {WorldEvent::Mute, "mute"},
  {WorldEvent::NoSmoke, "no_smoke"},
  {WorldEvent::Smoke, "smoke"},
  {WorldEvent::SimulationDisabled, "simulation_disabled"},
  {WorldEvent::SimulationEnabled, "simulation_enabled"},
});

#endif
