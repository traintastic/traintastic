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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_TRAINROUTEENTRYSOURCE_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_TRAINROUTEENTRYSOURCE_HPP

#include <cstdint>
#include <array>
#include "enum.hpp"

enum class TrainRouteEntrySource : uint8_t
{
  Explicit = 1,
  Resolver = 2,
};

TRAINTASTIC_ENUM(TrainRouteEntrySource, "train_route_entry_source", 2,
{
  {TrainRouteEntrySource::Explicit, "Explicit"},
  {TrainRouteEntrySource::Resolver, "Resolver"}
});

constexpr std::array<TrainRouteEntrySource, 2> trainRouteEntrySourceValues
{
  TrainRouteEntrySource::Explicit,
  TrainRouteEntrySource::Resolver,
};

#endif
