/**
 * server/src/hardware/output/map/turnoutoutputmapitem.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_OUTPUT_MAP_TURNOUTOUTPUTMAPITEM_HPP
#define TRAINTASTIC_SERVER_HARDWARE_OUTPUT_MAP_TURNOUTOUTPUTMAPITEM_HPP

#include "outputmapitembase.hpp"
#include <traintastic/enum/turnoutposition.hpp>

class TurnoutOutputMapItem : public OutputMapItemBase<TurnoutPosition>
{
  CLASS_ID("output_map_item.turnout")

  public:
    TurnoutOutputMapItem(Object& map, TurnoutPosition position);

    Property<bool> visible;
};

#endif
