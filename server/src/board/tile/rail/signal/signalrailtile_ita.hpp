/**
 * server/src/board/tile/rail/signal/signal3aspectrailtile.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_BOARD_TILE_RAIL_SIGNAL_SIGNALRAILTILE_ITA_HPP
#define TRAINTASTIC_SERVER_BOARD_TILE_RAIL_SIGNAL_SIGNALRAILTILE_ITA_HPP

#include "signalrailtile.hpp"

class SignalRailTileITA : public SignalRailTile
{
  CLASS_ID("board_tile.rail.signal_ita")
  CREATE(SignalRailTileITA)

  protected:
    void boardModified() final;

  public:
    SignalRailTileITA(World& world, std::string_view _id);

    void setAspectITA(SignalAspectITA value);

    void calculateLampStates();

    Property<SignalAspectITA> aspectITA;

    Property<SignalAspectITALampState> lampState1;
    Property<SignalAspectITALampColor> lampColor1;

    Property<SignalAspectITALampState> lampState2;
    Property<SignalAspectITALampColor> lampColor2;

    Property<SignalAspectITALampState> lampState3;
    Property<SignalAspectITALampColor> lampColor3;
};

#endif
