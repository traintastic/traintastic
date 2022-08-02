/**
 * server/src/board/map/connector.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#include "connector.hpp"

Connector::Connector(TileLocation location_, Direction direction_, Type type_)
  : location{location_}
  , direction{direction_}
  , type{type_}
{
}

Connector::Connector(TileLocation location_, TileRotate rotate, Type type_)
  : location{location_}
  , direction{toConnectorDirection(rotate)}
  , type{type_}
{
}

Connector Connector::opposite() const
{
  return {location + direction, ~direction, type};
}
