/**
 * shared/src/traintastic/board/tiledata.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2021,2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_BOARD_TILEDATA_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_BOARD_TILEDATA_HPP

#include "tileid.hpp"
#include "../enum/tilerotate.hpp"

struct TileData
{
  static constexpr uint8_t heightMax = 16;
  static constexpr uint8_t widthMax = 16;

  uint16_t _header;
  uint8_t _size;
  uint8_t state;

  inline TileData(TileId _id = TileId::None, TileRotate _rotate = TileRotate::Deg0, uint8_t _width = 1, uint8_t _height = 1, uint8_t state_ = 0) :
    _header{static_cast<uint16_t>((static_cast<uint16_t>(_id) << 4) | (static_cast<uint16_t>(_rotate) << 1) | (::isActive(_id) ? 1 : 0))},
    _size{0},
    state{state_}
  {
    setSize(_width, _height);
  }

  inline TileId id() const
  {
    return static_cast<TileId>(_header >> 4);
  }

  inline TileRotate rotate() const
  {
    return static_cast<TileRotate>((_header >> 1) & 0x0007);
  }

  inline void setRotate(TileRotate _rotate)
  {
    _header &= 0xFFF1;
    _header |= static_cast<uint16_t>(_rotate) << 1;
  }

  inline bool isActive() const
  {
    return (_header & 0x0001);
  }

  inline bool isPassive() const
  {
    return !isActive();
  }

  inline uint8_t width() const
  {
    return 1 + (_size & 0x0F);
  }

  inline uint8_t height() const
  {
    return 1 + (_size >> 4);
  }

  inline void setSize(uint8_t _width, uint8_t _height)
  {
    _size = ((_height - 1) << 4) | ((_width - 1) & 0x0F);
  }

  operator bool() const
  {
    return id() != TileId::None;
  }
};
static_assert(sizeof(TileData) == 4);

#endif
