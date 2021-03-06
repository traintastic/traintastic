/**
 * server/src/board/tile/tile.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2021 Reinder Feenstra
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

#include "tile.hpp"

Tile::Tile(const std::weak_ptr<World>& world, std::string_view _id, TileId tileId) :
  IdObject(world, _id),
  m_data{tileId}
{
}

void Tile::save(WorldSaver& saver, nlohmann::json& data, nlohmann::json& state) const
{
  IdObject::save(saver, data, state);

  data["x"] = m_location.x;
  data["y"] = m_location.y;
  data["rotate"] = toDeg(m_data.rotate());
  if(uint8_t height = m_data.height(); height > 1)
    data["height"] = height;
  if(uint8_t width = m_data.width(); width > 1)
    data["width"] = width;
}
