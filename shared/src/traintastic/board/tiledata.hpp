/**
 * shared/src/traintastic/board/tiledata.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020 Reinder Feenstra
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
#include "tilerotate.hpp"

struct TileData
{
  uint16_t _header;

  inline TileData(TileId _id = TileId::None, TileRotate _rotate = TileRotate::Deg0) :
    _header{static_cast<uint16_t>((static_cast<uint16_t>(_id) << 4) | (static_cast<uint16_t>(_rotate) << 1))}
  {
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

  inline bool isLong() const
  {
    return (_header & 0x0001);
  }

  operator bool() const
  {
    return id() != TileId::None;
  }
};
static_assert(sizeof(TileData) == 2);

struct TileDataLong : TileData
{
  uint8_t _size;
  uint8_t _reserved;

  TileDataLong(TileId _id = TileId::None, TileRotate _rotate = TileRotate::Deg0, uint8_t _width = 1, uint8_t _height = 1) :
    TileData(_id, _rotate),
    _size{0},
    _reserved{0}
  {
    setSize(_width, _height);
  }

  TileDataLong(const TileData& data) :
    TileData(data),
    _size{0},
    _reserved{0}
  {
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
    if(_size == 0) // short = 1x1
      _header &= 0xFFFE;
    else // long > 1x1
      _header |= 0x0001;
  }
};
static_assert(sizeof(TileDataLong) == 4);

#endif
