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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_AUTOTHROTTLEREASON_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_AUTOTHROTTLEREASON_HPP

#include <cstdint>
#include <array>
#include "enum.hpp"

enum class AutoThrottleReason : uint16_t
{
  None = 0,
  PowerOff = 1,
  WorldStop = 2,
  AutomaticDisabled = 3,
  NoRoute = 4,
  WaitingForPath = 5,
};

TRAINTASTIC_ENUM(AutoThrottleReason, "auto_throttle_reason", 6,
{
  {AutoThrottleReason::None, "none"},
  {AutoThrottleReason::PowerOff, "power_off"},
  {AutoThrottleReason::WorldStop, "world_stop"},
  {AutoThrottleReason::AutomaticDisabled, "automatic_disabled"},
  {AutoThrottleReason::NoRoute, "no_route"},
  {AutoThrottleReason::WaitingForPath, "waiting_for_path"},
});

constexpr std::array<AutoThrottleReason, 6> autoThrottleReasonValues
{
  AutoThrottleReason::None,
  AutoThrottleReason::PowerOff,
  AutoThrottleReason::WorldStop,
  AutoThrottleReason::AutomaticDisabled,
  AutoThrottleReason::NoRoute,
  AutoThrottleReason::WaitingForPath,
};

#endif
