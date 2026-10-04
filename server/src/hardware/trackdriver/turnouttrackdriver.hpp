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

#ifndef TRAINTASTIC_SERVER_HARDWARE_TRACKDRIVER_TURNOUTTRACKDRIVER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_TRACKDRIVER_TURNOUTTRACKDRIVER_HPP

#include "../../core/subobject.hpp"
#include "trackdriverconsumer.hpp"

class Train;

class TurnoutTrackDriver final
  : public SubObject
  , public TrackDriverConsumer
{
  CLASS_ID("turnout_track_driver")

public:
  TurnoutTrackDriver(Object& parent_, std::string_view parentPropertyName);

protected:
  void loaded() final;
  void worldEvent(WorldState worldState, WorldEvent worldEvent) final;
};

#endif
